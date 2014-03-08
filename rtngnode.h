#pragma once
#include "globals.h"
//#include "tcpcon.h"
#include "node.h"

class RoutingNode
{
public:
	int Initialize(const char* theManager);
	//NetworkConnection *myConnection;
	int mySocket;
	struct sockaddr_in server;
	struct sockaddr_in neighbor;

	map<int, struct Node> topology;
};