#include "Socket.h"
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

	while (1)
	{
		std::string r = s->ReceiveToChar(DELIMITER);
		if (r.empty()) break;

		std::string requestType;
		std::string requestCommand;
		std::string requestValues;

		std::string::size_type prev_pos = 0, pos = 0;
		int i = 0;
		while( (pos = r.find(' ', pos)) != std::string::npos )
		{
			std::string word( r.substr(prev_pos, pos - prev_pos) );
			if(i == 0)
				requestType = word;
			else if(i == 1)
			{
				requestCommand = word;
				break;
			}
			prev_pos = ++pos;
			i++;
		}
		requestValues = r.substr(pos, r.length());
		std::cout << requestType << requestCommand << requestValues << '\n';

		switch(s_mapRequestTypes[requestType])
		{
			/*case "REQ":
				// TODO: Check if ip already exists in socket_list
				s->SendBytes("OK" + DELIMITER);
				break;
			default:
				s->SendBytes("ERR unkown command" + DELIMITER);
				break;*/
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
