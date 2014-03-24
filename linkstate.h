#pragma once
#include "globals.h"
#include "rtngnode.h"
#include <set>
#include <limits>
#include <queue>

typedef map<int, map<int,int>, less<int> >::iterator topologyIter;

class LinkStateNode : public RoutingNode
{
public:
	LinkStateNode (){lspCount = 0; topologyUpdated = false;}
	virtual bool ProcessMessages();
	virtual int InitializeForwardingTableConnections();
	int GenerateLSP();
	int FloodLSPs();
	int Dijkstra(map<int,map<int, int> > topo);
	int RebuildForwardingTable();
public:
	//node | neighbor, cost
	map<int, map<int,int> > topology;

	map<int,int> distance;
	map<int,int> previous;

	//holds distance values of neighbors
	map<int,int> distanceMap;

	//holds path to neighbors
	map<int,int> pathMap;

	vector<string> lsPackets;
	int lspCount;
	bool topologyUpdated;
};