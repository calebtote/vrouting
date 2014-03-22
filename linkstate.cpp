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
				//just some interesting info
				lspCount++;

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
					cout << "\t\t*** LSP Received! ***\n\t\t---Count: " << lspCount << endl;
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

int
LinkStateNode::InitializeForwardingTableConnections()
{
	char buffer[512];

	/* Temporary fix. 
	/* This should be abstracted, and more widely used
	/********* Start Select() ***********/
	fd_set readfds;
	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;	//wait for 2 second here

	int rv = 1;

	// clear the set ahead of time
	FD_ZERO(&readfds);

	// add our descriptors to the set
	FD_SET(mySocket, &readfds);

	// the n param for select()
	int n = mySocket + 1;
	/********* End Select() ***********/

	while(rv != 0)
	{
		rv = select(n, &readfds, NULL, NULL, &tv);

		if (rv == -1)
			perror("select"); // error occurred in select()

		bzero(buffer,512);
		int n = recvfrom(mySocket,buffer,512,0,(struct sockaddr *)&neighbor, &sockLen);
		if (n > 0)
		{
			parser.ParseMessage(buffer, fromNode, messages);
			ProcessMessages();
		}
	}

	Dijkstra(topology);

	char converged[20];
	sprintf(converged, "@%d~%d~ ~", myID, RoutingMessage::CONVERGED);
	SendMessage(server, converged);

	return 0;
}

//Map is formated as:
//<Node, Their Neighbor, Link Cost>
int 
LinkStateNode::Dijkstra(map<int,map<int, int> > topo)
{
	map<int,map<int, int> >::iterator it = topo.begin();
	#if logging > 2
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
	#endif

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
		it++;
	}

	distance[myID] = 0;

	neighborsIter nbrIt = neighbors.begin();
	while(nbrIt != neighbors.end())
	{
		distance[nbrIt->first] = nbrIt->second;
		previous[nbrIt->first] = myID;
		nbrIt++;
	}

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

		#if logging > 2
			cout << "Current Node: " << curNode << " -- wtih path cost -- " << minVal << endl;
		#endif

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
					
					#if logging > 2
						cout << "\t->Neighbor: " << t->second.begin()->first << " with link cost: " << x << endl;
					#endif

					if(x < distance[inner_it->first])
					{
						distance[inner_it->first] = x;
						previous[inner_it->first] = curNode;

						#if logging > 2						
							cout << "\t-->Distance to {" << inner_it->first << "} : " << distance[inner_it->first] << endl;
							cout << "\t\t->Previous Hop {" << previous[inner_it->first] << "}\n";
						#endif
					}
				//	cin.get();
					inner_it++;
				}
			}
		}
	}

	vector<int> path;
	int p = 3701;
	path.push_back(3701);
	while(p != myID)
	{
		cout << "previous[" << p << "]: " << previous[p] << endl;
		p = previous[p];
		path.push_back(p);
		//cin.get();
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

	/* Dijkstra Test /*
	map<int, map<int,int> > topology;
	
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

	LinkStateNode *lsNode = new LinkStateNode();
	lsNode->Dijkstra(topology);
	return 0;
	/**/

	LinkStateNode *lsNode = new LinkStateNode();

	lsNode->Initialize(argv[1]);	
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
	lsNode->RequestNeighborConnectionInfo();
	lsNode->GenerateLSP();
	lsNode->FloodLSPs();
	lsNode->InitializeForwardingTableConnections();
   	lsNode->Listen();

	return 0;
	/**/
}