#pragma once
#include "globals.h"
//#include "tcpcon.h"
#include "node.h"
#include "rtngmsg.h"

typedef map<int,int, less<int> >::iterator neighborsIter;
typedef map<int, map<int,int>, less<int> >::iterator forwardingTableIter;

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

	int GetNeighborLinkCost(int id);
	bool IsBetterPath(int destID, int destCost);
	int UpdateForwardingTable(int dest, int hop, int cost);
	int SendForwardingTableToNeighbors();
	int Broadcast(char* buffer);
	int WaitForAllClear();
	int Listen();

	int myID;
	int mySocket;
	int neighborSocket;
	struct sockaddr_in server;
	struct sockaddr_in neighbor;

	map<int, struct Node> topology;
	map<int, map<int,int> > forwardingTable;

	//this nodes known neighbors
	//Neighbors:  <Destination,Cost>
	std::map<int,int> neighbors;
	
	int fromNode;
	multimap<int, string> messages;
	RoutingMessage parser;

	socklen_t sockLen = sizeof(struct sockaddr_in);
};