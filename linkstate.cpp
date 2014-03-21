#include "linkstate.h"

bool
LinkStateNode::ProcessMessages()
{
	// fromNode has already been set, when the message
	// was first parsed. Probably need to fix that.

	messagesIter iter = messages.begin();
	unsigned int length = sizeof(struct sockaddr_in);
	while(iter != messages.end())
	{
		switch(iter->first)
		{
			case RoutingMessage::LSP: 
			{
				int node, neighbor, cost;
				string buf(iter->second);
				string nodeCostDelim = ".";	
				string nodeViewDelim = "|";
				string endNodeViewDelim = "%";

				//LSP Format:
				//@SentFromNode~LSP~NodeCView|nodeA.cost.nodeB.cost.%NodeBView|<..>
				if(buf.find(nodeCostDelim) == -1)
				{
					perror("!# LSP malformed");
					return false;
				}

				do
				{
					node = atoi(buf.substr(0, buf.find(nodeViewDelim)).c_str());
					buf.erase(0, buf.find(nodeViewDelim) + nodeViewDelim.length());

					#if logging > 2
						cout << "On Node: " << node << endl;
					#endif

					while(buf.find(nodeCostDelim) != -1 && buf[0] != '%')
					{
						neighbor = atoi(buf.substr(0, buf.find(nodeCostDelim)).c_str());
						buf.erase(0, buf.find(nodeCostDelim) + nodeCostDelim.length());
						cost = atoi(buf.substr(0, buf.find(nodeCostDelim)).c_str());
						buf.erase(0, buf.find(nodeCostDelim) + nodeCostDelim.length());

						if(topology[node][neighbor] != cost)
							topologyUpdated = true;

						topology[node][neighbor] = cost;
						#if logging > 2
							cout << "topology[" << node << "][" << neighbor << "] = " << cost << endl;
						#endif
					}

					//remove the '%'
					buf.erase(buf.begin());

				}while(buf.find(nodeViewDelim) != -1);

				#if logging > 1
					cout << "\t\t*** LSP Received! ***\n";
					if(topologyUpdated)
					{
						cout << "\n\t*** Printing Topology ***\n";
						topologyIter topoIt = topology.begin();
						while(topoIt != topology.end())
						{
							cout << "\tNode: " << topoIt->first << " connected to:\n";
							map<int,int, less<int> >::iterator inner_it = topoIt->second.begin();
							while(inner_it != topoIt->second.end())
							{
								cout << "\t\t--" << inner_it->first << " with cost: " << inner_it->second << endl;
								inner_it++;
							}
							topoIt++;
						}

					}
				#endif

				if(topologyUpdated)
				{
					GenerateLSP();
					FloodLSPs();
					topologyUpdated = false;
				}

			}break;
			case RoutingMessage::NEWNODE: 
			{
				myID = atoi(iter->second.c_str());
				BindSocketToPort();
				UpdateForwardingTable(myID, myID, 0);
				topology[myID][myID] = 0;
			} break;
			case RoutingMessage::LINKADD: 
			{
				int destID, destCost;
				string buf(iter->second);
				string delim = ".";	
				destID = atoi(buf.substr(0, buf.find(delim)).c_str());
				buf.erase(0, buf.find(delim) + delim.length());
				destCost = atoi(buf.substr(0, buf.find(delim)).c_str());

				cout << "now linked to node " << destID-3700 << " with cost " << destCost << endl;

				neighbors.insert(pair<int,int>(destID, destCost));
				topology[myID][destID] = destCost;
				UpdateForwardingTable(destID, destID, destCost);
			} break;
			default: RoutingNode::ProcessMessages();
		}
		iter++;
	}

	messages.clear();
	forwardingTableUpdated = false;
}

int 
LinkStateNode::GenerateLSP()
{
	//LSP Format:
	//@SentFromNode~LSP~NodeCView|nodeA.cost.nodeB.cost%NodeBView|nodeA.cost.nodeB.cost <...>
	char buffer[512];
	sprintf(buffer, "@%d~%d~", myID, RoutingMessage::LSP);

	topologyIter topoIt = topology.begin();
	while(topoIt != topology.end())
	{
		char node[5];
		sprintf(node, "%d|", topoIt->first);
		strcat(buffer,node);

		map<int,int, less<int> >::iterator inner_it = topoIt->second.begin();
		while(inner_it != topoIt->second.end())
		{
			char temp[20];
			sprintf(temp, "%d.%d.", inner_it->first, inner_it->second);
			strcat(buffer,temp);

			inner_it++;
		}

		// Add '%' to denote end of node view
		strcat(buffer,"%");

		#if logging > 2
			cout << "Buffer after {" << topoIt->first << "}: " << buffer << endl;
		#endif

		topoIt++;
	}

	lsPackets.push_back(buffer);
	return 0;
}

int
LinkStateNode::FloodLSPs()
{
	while(lsPackets.size() > 0)
	{
		string lsp = lsPackets.back();
		char *cstr = new char[lsp.length() + 1];
		strcpy(cstr, lsp.c_str());
		Broadcast(cstr);
		lsPackets.pop_back();
		delete []cstr;
	}
}

//Map is formated as:
//<Node, Their Neighbor, Link Cost>
int Dijkstra(multimap<int,map<int, int> > topo, int source)
{
	multimap<int,map<int, int> >::iterator it = topo.begin();
	while(it != topo.end())
	{
		cout << "\tNode: " << it->first << " connected to:";
		map<int,int, less<int> >::iterator inner_it = it->second.begin();
		while(inner_it != it->second.end())
		{
			cout << "--" << inner_it->first << " with cost: " << inner_it->second << endl;
			inner_it++;
		}
		it++;
	}

	map<int,int> distance;
	map<int,int> previous;

	map<int,int>::iterator distIt;
	map<int,int>::iterator prevIt;

	set<int> nodes;

	it = topo.begin();
	while(it != topo.end())
	{
		distance[it->first] = 9999;
		previous[it->first] = -1;
		nodes.insert(it->first);
		cout << "Processed: " << it->first << endl;
		it++;
	}

	distance[source] = 0;
	distance[101] = 8;
	distance[105] = 4;
	distance[103] = 3;
	previous[101] = 102;
	previous[105] = 102;
	previous[103] = 102;

	while(!nodes.empty())
	{
		int minVal = 9999;
		int curNode = -1;
		for(distIt = distance.begin(); distIt != distance.end(); distIt++)
		{
			cout << "distit:" << distIt->first << ":" << distIt->second << endl;
			if(nodes.find(distIt->first) != nodes.end() && distIt->second < minVal)
			{
				minVal = distIt->second;
				curNode = distIt->first;
			}
		}

		cout << "MIN: " << curNode << "|wtih cost|" << minVal << endl;
		nodes.erase(curNode);

		if(distance.at(curNode) == 9999)
			break;

		typedef multimap<int,map<int,int> >::iterator tmpIt;
		tmpIt t = topo.begin();
		
		for(; t != topo.end(); t++)
		{
			if(t->first == curNode)
			{
				map<int,int, less<int> >::iterator inner_it = t->second.begin();
				while(inner_it != t->second.end())
				{
					int x = distance[curNode] + inner_it->second;
					cout << "node: " << t->second.begin()->first << " with total cost: " << x << endl;
					if(x < distance[inner_it->first])
					{
						distance[inner_it->first] = x;
						previous[inner_it->first] = curNode;						
						cout << "distance to {" << inner_it->first << "} --> " << distance[inner_it->first] << endl;
						cout << "previous hop {" << previous[inner_it->first] << "}\n";
					}
					cin.get();
					inner_it++;
				}
			}
		}
	}

	vector<int> path;
	int p = 101;
	path.push_back(101);
	while(p != source)
	{
		cout << "previous[" << p << "]: " << previous[p] << endl;
		p = previous[p];
		path.push_back(p);
		cin.get();
	}

	vector<int>::reverse_iterator pIter = path.rbegin();
	while(pIter != path.rend())
	{
		cout << "Hop: {" << *pIter << "} --\n";
		pIter++;
	}

}


int main(int argc, const char* argv[])
{

	/* Dijkstra Test 
	multimap<int, map<int,int> > topology;
	
	//Node's neighbor | cost
	map<int, int> t;
	t.insert(pair<int,int>(102,8));
	topology.insert(pair<int, map<int, int> >(101, t));
	t.clear();

	t.insert(pair<int,int>(104,1));
	topology.insert(pair<int, map<int, int> >(101, t));
	t.clear();

	t.insert(pair<int,int>(101,8));
	topology.insert(pair<int, map<int, int> >(102, t));
	t.clear();

	t.insert(pair<int,int>(105,4));
	topology.insert(pair<int, map<int, int> >(102, t));
	t.clear();

	t.insert(pair<int,int>(103,3));
	topology.insert(pair<int, map<int, int> >(102, t));
	t.clear();

	t.insert(pair<int,int>(102,3));
	topology.insert(pair<int, map<int, int> >(103, t));
	t.clear();

	t.insert(pair<int,int>(101,1));
	topology.insert(pair<int, map<int, int> >(104, t));
	t.clear();

	t.insert(pair<int,int>(105,1));
	topology.insert(pair<int, map<int, int> >(104, t));
	t.clear();

	t.insert(pair<int,int>(104,1));
	topology.insert(pair<int, map<int, int> >(105, t));
	t.clear();

	t.insert(pair<int,int>(102,4));
	topology.insert(pair<int, map<int, int> >(105, t));
	t.clear();

	Dijkstra(topology, 102);
	return 0;
	*/

	LinkStateNode *lsNode = new LinkStateNode();

	lsNode->Initialize("localhost");	
	lsNode->GetMyID();
	lsNode->GetMyNeighbors();

	#if logging > 1		
		neighborsIter it = lsNode->neighbors.begin();
		while (it != lsNode->neighbors.end())
		{
			cout << "Connection to: " << it->first << " with cost " << it->second << endl;
			it++;
		}
	#endif

	lsNode->WaitForAllClear();
	lsNode->GenerateLSP();
	lsNode->FloodLSPs();

   	lsNode->Listen();

	return 0;
}