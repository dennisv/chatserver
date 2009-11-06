#include "Socket.h"
#include "imexplode.h"
#include "config.h"
#include <process.h>
#include <iostream>
#include <string>
#include <list>
#include <map>

struct chatclient
{
	std::string address;
	std::string nickname;
	std::string status;

	chatclient(std::string a, std::string n, std::string s)
	{
		address = a;
		nickname = n;
		status = s;
	}
	chatclient() { }
};

typedef std::list<Socket*> socket_list;
typedef std::map<std::string, struct chatclient*> client_list;

static std::map<std::string, RequestTypes> s_mapRequestTypes;
static std::map<std::string, RequestCommands> s_mapRequestCommands;

socket_list g_connections;
client_list g_clients;

static void initialize()
{
	s_mapRequestTypes["REQ"] = Request;
	s_mapRequestTypes["SND"] = Send;
	s_mapRequestTypes["GET"] = Get;
	s_mapRequestTypes["SET"] = Set;

	s_mapRequestCommands["CONTACTS"] = Contacts;
	s_mapRequestCommands["STATUS"] = Status;
	s_mapRequestCommands["NICKNAME"] = Nickname;
	s_mapRequestCommands["JOIN"] = Join;
	s_mapRequestCommands["LEAVE"] = Leave;
}

static void printEvent(struct chatclient* c, std::string text)
{
	std::string clientname;
	clientname = c->address;
	if(c->nickname != "")
		clientname += " (" + c->nickname + ")";
	std::string out = clientname + " " + text;
	std::cout << out << std::endl;
}

static bool isClient(std::string address)
{
	return g_clients.find(address) != g_clients.end();
}

static std::string getContactList(std::string address)
{
	std::string contacts = "XML <?xml version=\"1.0\" encoding=\"UTF-8\"?><contacts>";
	for(std::map<std::string, struct chatclient*>::const_iterator ic = g_clients.begin();
			ic != g_clients.end();
			ic++)
	{
		std::string name = ic->second->nickname;
		if(ic->second->nickname == "")
			name = ic->second->address;
		if(ic->second->address != address)
			contacts += "<contact ip=\"" + ic->second->address + "\" name=\"" + name + "\" status=\"" + ic->second->status + "\" />";
	}
	contacts += "</contacts>";
	return contacts;
}

static std::string rtrim(std::string in)
{
	return in.erase(in.find_last_not_of(" \t\r\n") + 1);
}

unsigned __stdcall Connection(void* a)
{
	Socket* s = (Socket*) a;
	std::string address;

	g_connections.push_back(s);

	address = s->Address();
	std::cout << address + " Connected" << std::endl;

	while (1)
	{
		std::string r = s->ReceiveToChar(DELIMITER);
		if (r.empty()) break;

		r = rtrim(r);
		std::string requestType;
		std::string requestCommand;
		std::string requestValues;

		std::vector<std::string> commands = explode(' ', r);
		if(commands.size() < 2) {
			s->SendDelimiter("ERR unknown command", DELIMITER);
			continue;
		}
		requestType = commands[0];
		commands.erase(commands.begin());
		requestCommand = commands[0];
		commands.erase(commands.begin());
		requestValues = implode(' ', commands);

		bool hasValue = requestValues.length() > 0;

		std::cout << "-- DEBUG: " + address + " <" + requestType + "> <" + requestCommand + "> <" + requestValues + ">" << std::endl;
		switch(s_mapRequestTypes[requestType])
		{
			case Request:
				switch(s_mapRequestCommands[requestCommand])
				{
					case Join:
						if(!isClient(address))
						{
							g_clients[address] = new chatclient(address, "", "online");
							s->SendDelimiter("OK", DELIMITER);
							printEvent(g_clients[address], "joined contactlist");

							std::string contacts;
							contacts = getContactList(address);
							s->SendDelimiter(contacts, DELIMITER);
							printEvent(g_clients[address], "sent contactlist");
						}
						else
						{
							std::cout << address + " tried to connect again" << std::endl;
							s->SendDelimiter("ERR already connected", DELIMITER);
							s->Close();
							g_connections.remove(s);
							delete s;
							return 0;
						}
						break;
					default:
						s->SendDelimiter("ERR unknown command (" + requestCommand + ")", DELIMITER);
						break;
				}
				break;
			case Get:
				switch(s_mapRequestCommands[requestCommand])
				{
					case Contacts:
					{
						if(!isClient(address))
						{
							s->SendDelimiter("ERR need to join first", DELIMITER);
							break;
						}
						std::string contacts;
						contacts = getContactList(address);
						s->SendDelimiter(contacts, DELIMITER);
						printEvent(g_clients[address], "requested contactlist");
					}
					break;
					default:
						s->SendDelimiter("ERR unknown command (" + requestCommand + ")", DELIMITER);
						break;
				}
				break;
			case Set:
				switch(s_mapRequestCommands[requestCommand])
				{
					case Nickname:
						if(!isClient(address))
						{
							s->SendDelimiter("ERR need to join first", DELIMITER);
							break;
						}
						if(hasValue)
						{
							g_clients[address]->nickname = requestValues;
							s->SendDelimiter("OK", DELIMITER);
							printEvent(g_clients[address], "set nickname");
						}
						break;
					case Status:
						if(!isClient(address))
						{
							s->SendDelimiter("ERR need to join first", DELIMITER);
							break;
						}
						if (hasValue)
						{
							g_clients[address]->status = requestValues;
							s->SendDelimiter("OK", DELIMITER);
							printEvent(g_clients[address], "set status");
						}
						break;
					default:
						s->SendDelimiter("ERR unknown command (" + requestCommand + ")", DELIMITER);
						break;
				}
				break;
			case Send:
				switch(s_mapRequestCommands[requestCommand])
				{
					case Leave:
						if(isClient(address))
						{
							printEvent(g_clients[address], "sent leave");
							g_clients.erase(address);
						}
						s->Close();
						g_connections.remove(s);
						delete s;
						return 0;
				}
				break;
			default:
				s->SendDelimiter("ERR unknown command (" + requestType + ")", DELIMITER);
				break;
		}
	}

	std::cout << address + " disconnected" << std::endl;
	g_clients.erase(address);
	g_connections.remove(s);

	delete s;

	return 0;
}

int main()
{
	initialize();

	SocketServer in(5776, 50);
	std::cout << "Server started and listening on port 5776" << std::endl;

	while (1) {
		Socket* s = in.Accept();

		unsigned ret;
		_beginthreadex(0, 0, Connection, (void*) s, 0, &ret);
	}

	return 0;
}
