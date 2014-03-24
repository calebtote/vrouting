#pragma once
#include "globals.h"
//#include "tcpcon.h"
#include "node.h"
#include "rtngmsg.h"
#include <set>

typedef map<int,int, less<int> >::iterator neighborsIter;
typedef map<int, map<int,int>, less<int> >::iterator forwardingTableIter;
typedef map<int, map<int,int>, less<int> >::iterator neighborVectorsIter;
typedef multimap<int,string>::iterator messagesToSendIter;

class RoutingNode
{
public:
	RoutingNode(){forwardingTableUpdated = false; neighborVectorsUpdated = false;}
	int Initialize(const char* theManager);
	int SendMessage(struct sockaddr_in toNode, char buffer[512]);
	//NetworkConnection *myConnection;
	int GetMyID();
	int GetMyNeighbors();
	int BindSocketToPort();
	virtual bool ProcessMessages();
	int AppendMyID(string &theMessage);
	int PrintMessage(string theMessage);

	int GetNeighborLinkCost(int id);
	bool IsNewNode(int destID);
	bool IsBetterPath(int destID, int destCost);
	int RequestVectorUpdates();
	int UpdateNeighborVector(int neighbor, int node, int cost);
	int UpdateForwardingTable(int dest, int hop, int cost);
	int RebuildForwardingTable();
	int SendForwardingTableToNeighbors();
	int Broadcast(char* buffer);
	int WaitForAllClear();
	int RequestNeighborConnectionInfo();
	virtual int InitializeForwardingTableConnections();
	int Listen();

	int myID;
	int mySocket;
	int neighborSocket;
	struct sockaddr_in server;
	struct sockaddr_in neighbor;

	//destination | hop, cost
	map<int, map<int,int> > forwardingTable;

	//neighbor | node, cost
	map<int, map<int,int> > neighborVectors;

	//this nodes known neighbors
	//Neighbors:  <Destination,Cost>
	map<int,int> neighbors;
	map<int,string> neighborConnectionInfo;

	set<int> knownNodes;
	
	int fromNode;
	bool forwardingTableUpdated;
	bool neighborVectorsUpdated;

	multimap<int, string> messages;
	multimap<int, string> messagesToSend;
	RoutingMessage parser;

	socklen_t sockLen = sizeof(struct sockaddr_in);
};