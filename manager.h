#pragma once

#include "globals.h"

//#include "tcpcon.h"
#include "node.h"
#include "rtngmsg.h"
#include <fstream>
#include <list>

typedef map<int,Node, less<int> >::iterator TopologyIter;
typedef vector<sockaddr_in>::iterator activeNodesIter;

bool operator == (const sockaddr_in &lhs, const sockaddr_in& rhs) {
      return lhs.sin_family == rhs.sin_family 
        && lhs.sin_port == rhs.sin_port
        && lhs.sin_addr.s_addr == rhs.sin_addr.s_addr;
       // && lhs.sin_zero == rhs.sin_zero;
}
bool operator != (const sockaddr_in &lhs, const sockaddr_in& rhs) {
      return !(lhs == rhs);
}

class RoutingManager
{
public:
	static const int MAX_TOKENS = 5;
	static const int MAX_NODES = 100;

	RoutingManager();
	int Initialize(int myPort = SERVER_PORT);
	int WaitForNewNodes();
	int Run();
	int Listen();


	void GenerateConnectionString(int id, char* buf);
	void SendNeighborInformation(struct Node n, char* buffer);
	int SendMessage(struct sockaddr_in toNode, char buffer[1024]);
	int GenerateVirtualNodeID(int inputNodeID);
	int ProcessMessages();
	
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
	//map<int, struct Node> activeTopology;
	list<int> availableIDs;

	int fromNode;
	multimap<int, string> messages;
	socklen_t sockLen = sizeof(struct sockaddr_in);
};
