#pragma once

#include <map>
#include "nodecon.h"
typedef map<int,int, less<int> >::const_iterator NodeNeighborsIter;

struct Node
{
	Node(){online = false; converged = false;}
	int id;
	bool online;
	bool converged;

	//this nodes known neighbors
	std::map<int,int> neighbors;

	//this nodes connection information
	struct NodeConnection connection;
};