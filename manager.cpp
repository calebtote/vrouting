#include "manager.h"

RoutingManager::RoutingManager()
{

}

int
RoutingManager::GenerateVirtualNodeID(int inputNodeID)
{
	/* Testing is on a single machine, so we're going to use
	 the virtual ID as both the Node ID and the Port we'll
	 listen on.

	 VirtualNodeID is comprised of:
	    nodeId from <input file> + 3700
	    Example: for Node 4 our ID will be 3704
	             and we will also listen on port 3704  */
	return inputNodeID+3700;
}

int
RoutingManager::ParseInputFile(char* filePath, const int MAX_CHARS_PER_LINE,
							    const int MAX_TOKENS_PER_LINE, const char* const DELIMITER)
{
	ifstream theFile;
	theFile.open(filePath);

	if(!theFile.good())
	{
		cout << "Input File Not Found.\n";
		return 1;
	}

	do
	{
		char buf[MAX_CHARS_PER_LINE];
		theFile.getline(buf, MAX_CHARS_PER_LINE);

		char* temp = strtok(buf, DELIMITER);
		while(temp != NULL)
		{
			string val(temp);
			tokens.push_back(val);
			temp = strtok(NULL, DELIMITER);
		}
	}while (!theFile.eof());



	return 0;
}

int
RoutingManager::AddNodeLink(int nodeID, int destID, int destCost)
{
	if(topology.find(nodeID) == topology.end())
	{
		Node n = Node();
		n.id = nodeID;
		n.neighbors.insert(pair<int,int>(destID, destCost));

		topology.insert(pair<int, Node>(nodeID, n));

		#if logging > 0
			cout << "\nNode Added: " << n.id << endl;
			cout << "Neighbor {" << destID << "} added to {" << nodeID 
				<< "}, with cost {" << n.neighbors.at(destID) << "}\n";
		#endif
	}
	else
	{
		topology.at(nodeID).neighbors.insert(pair<int,int>(destID, destCost));

		#if logging > 0
			cout << "Neighbor {" << destID << "} added to {" << nodeID 
				<< "}, with cost {" << topology.at(nodeID).neighbors.at(destID) << "}\n";
		#endif
	}
}

int
RoutingManager::PopulateTopology()
{
	/* 
	   Assuming valid input for now; Tokens are stored as:
	     [0]: From Node
	     [1]: To Node
	     [2]: Link Cost
	     ... etc.
	*/
	for (int i = 0; i < tokens.size(); i+=3)
	{
		int id;
		int dest;
		int cost;

		istringstream(tokens[i]) >> id;
		istringstream(tokens[i+1]) >> dest;
		istringstream(tokens[i+2]) >> cost;

		AddNodeLink(GenerateVirtualNodeID(id), dest, cost);
	}
}

int
RoutingManager::PrintTopology()
{
	cout << "Topology Size: " << topology.size() << endl;

	TopologyIter topoIter = topology.begin();
	for (; topoIter != topology.end(); topoIter++)
	{
		cout << "Table for {" << topoIter->first << "}:\n";

		NodeNeighborsIter NeighborIter = topoIter->second.neighbors.begin();
		for (; NeighborIter != topoIter->second.neighbors.end(); NeighborIter++)
			cout << "\tConnected to: {" << NeighborIter->first << "} with cost {" << NeighborIter->second << "}\n";
	}
}


/************************************************/
/*                 MAIN                         */
/************************************************/
int main(int argc, const char* argv[])
{
	TCPConnection *tcpcon = new TCPConnection("localhost", "7777");
	tcpcon->SetHints();
	tcpcon->PopulateAddressInfo();
	tcpcon->BindSocket();
	cout << "\n\t**** **** ****\n";

	RoutingManager *manager = new RoutingManager();
	manager->ParseInputFile("topo.txt", 10, 3, " ");

	#if logging > 1
		// print the tokens
	    for (int i = 0; i < manager->tokens.size(); i++) // n = #of tokens
	      cout << "Token[" << i << "] = " << manager->tokens[i] << endl;
	    cout << "\n\t**** **** ****\n";
    #endif

	manager->PopulateTopology();
	cout << "\n\t**** **** ****\n";

	#if logging > 0
		manager->PrintTopology();
		cout << "\n\t**** **** ****\n";
	#endif

	cout << "\n\t****  End.  ****\n";

	return 0;
}