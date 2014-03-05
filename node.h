#pragma once

#include <map>
#include "nodecon.h"
typedef map<int,int, less<int> >::const_iterator NodeNeighborsIter;

struct Node
{
	int id;
	std::map<int,int> neighbors;
	bool online;

	struct NodeConnection connection;
};