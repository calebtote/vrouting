#pragma once

#include "globals.h"

//#include "tcpcon.h"
#include "node.h"
#include <fstream>
#include <list>

typedef map<int,Node, less<int> >::iterator TopologyIter;
typedef vector<sockaddr_in>::iterator activeNodesIter;

class RoutingManager
{
public:
	static const int MAX_TOKENS = 5;
	static const int MAX_NODES = 100;

	RoutingManager();
	int Initialize(int myPort = 7777);
	int WaitForNewNodes();
	int SendMessage(struct sockaddr_in toNode, char buffer[1024]);
	int GenerateVirtualNodeID(int inputNodeID);
	int ParseInputFile(char* filePath, const int MAX_CHARS_PER_LINE = 512, 
							    const int MAX_TOKENS_PER_LINE = 20, const char* const DELIMITER = " ");
	int AddNodeLink(int nodeID, int destNodeId, int destLinkCost);
	bool ActivateNewNode(struct sockaddr_in newNode);
	int PopulateTopology();
	int PrintTopology();

public:
	//NetworkConnection *myConnection;
	int mySocket;

	char* topoFilePath;
	char* msgFilePath;
	vector<string> tokens;
	vector<sockaddr_in> activeNodes;

	size_t activeNodeCount;

	map<int, struct Node> topology;
	map<int, struct Node> activeTopology;
	list<int> availableIDs;
};
