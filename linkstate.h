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
	virtual bool ProcessMessages();
	int GenerateLSP();
	int FloodLSPs();
public:
	//node | neighbor, cost
	map<int, map<int,int> > topology;

	vector<string> lsPackets;
};