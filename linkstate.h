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
	LinkStateNode (){lspCount = 0;}
	virtual bool ProcessMessages();
	virtual int InitializeForwardingTableConnections();
	int GenerateLSP();
	int FloodLSPs();
	int Dijkstra(map<int,map<int, int> > topo);
public:
	//node | neighbor, cost
	map<int, map<int,int> > topology;

	vector<string> lsPackets;
	int lspCount;
};