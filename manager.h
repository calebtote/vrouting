#pragma once

#include "globals.h"

#include "tcpcon.h"
#include "node.h"
#include <fstream>
#include <list>

typedef map<int,Node, less<int> >::const_iterator TopologyIter;

class RoutingManager
{
public:
	static const int MAX_TOKENS = 5;
	static const int MAX_NODES = 100;

	RoutingManager();
	int GenerateVirtualNodeID(int inputNodeID);
	int ParseInputFile(char* filePath, const int MAX_CHARS_PER_LINE = 512, 
							    const int MAX_TOKENS_PER_LINE = 20, const char* const DELIMITER = " ");
	int AddNodeLink(int nodeID, int destNodeId, int destLinkCost);
	int PopulateTopology();
	int PrintTopology();

public:
	char* topoFilePath;
	char* msgFilePath;
	vector<string> tokens;

	map<int, struct Node> topology;
	list<int> availableIDs;
};