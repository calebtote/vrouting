#pragma once
#include "globals.h"
//#include "tcpcon.h"
#include "node.h"
#include "rtngmsg.h"

typedef map<int,string, less<int> >::iterator messagesIter;

class RoutingNode
{
public:
	int Initialize(const char* theManager);
	//NetworkConnection *myConnection;
	int GetMyID();
	int CreateNeighborSocket();
	int ProcessMessages();
	int myID;
	int mySocket;
	int neighborSocket;
	struct sockaddr_in server;
	struct sockaddr_in neighbor;

	map<int, struct Node> topology;
	
	int fromNode;
	map<int, string> messages;
	RoutingMessage parser;
};