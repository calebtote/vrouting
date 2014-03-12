#pragma once
#include "globals.h"
//#include "tcpcon.h"
#include "node.h"
#include "rtngmsg.h"

typedef map<int,string, less<int> >::iterator messagesIter;
typedef map<int,int, less<int> >::iterator neighborsIter;

class RoutingNode
{
public:
	int Initialize(const char* theManager);
	int SendMessage(struct sockaddr_in toNode, char buffer[512]);
	//NetworkConnection *myConnection;
	int GetMyID();
	int GetMyNeighbors();
	int BindSocketToPort();
	int CreateNeighborSocket();
	int ProcessMessages();
	int myID;
	int mySocket;
	int neighborSocket;
	struct sockaddr_in server;
	struct sockaddr_in neighbor;

	map<int, struct Node> topology;

	//this nodes known neighbors
	std::map<int,int> neighbors;
	
	int fromNode;
	multimap<int, string> messages;
	RoutingMessage parser;
};