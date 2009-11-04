#include "Socket.h"
#include "imexplode.h"
#include "config.h"
#include <process.h>
#include <iostream>
#include <string>
#include <list>
#include <map>

typedef std::list<Socket*> socket_list;

static std::map<std::string, RequestTypes> s_mapRequestTypes;
static std::map<std::string, RequestCommands> s_mapRequestCommands;

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

socket_list g_connections;
socket_list g_clients;

unsigned __stdcall Connection(void* a)
{
	Socket* s = (Socket*) a;

	g_connections.push_back(s);

	std::cout << "Connected" << '\n';

	s->SendDelimiter("XML <?xml version=\"1.0\" encoding=\"UTF-8\"?><contacts><contact ip=\"127.0.0.1\" name=\"Jordi\" status=\"online\" /><contact ip=\"127.0.0.2\" name=\"Rein\" status=\"online\" /></contacts>", DELIMITER);

	while (1)
	{
		std::string r = s->ReceiveToChar(DELIMITER);
		if (r.empty()) break;

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

		std::cout << requestType << requestCommand << requestValues << '\n';

		std::cout << r << '\n';
		switch(s_mapRequestTypes[requestType])
		{
			case Request:
				// TODO: Check if ip already exists in socket_list
				s->SendBytes("OK" + DELIMITER);
				break;
			default:
				s->SendDelimiter("ERR unknown command", DELIMITER);
				break;
		}

		for (socket_list::iterator os = g_connections.begin();
				os!=g_connections.end(); 
				os++)
		{
			if (*os != s) (*os)->SendLine(r);
		}
	}

	g_connections.remove(s);

	delete s;

	return 0;
}

int main()
{
	initialize();

	SocketServer in(5776, 50);

	while (1) {
		Socket* s = in.Accept();

		unsigned ret;
		_beginthreadex(0, 0, Connection, (void*) s, 0, &ret);
	}

	return 0;
}
