#include "distvec.h"

bool
DistanceVectorNode::ProcessMessages()
{
	if(RoutingNode::ProcessMessages())
		return true;

	if(forwardingTableUpdated)
		SendForwardingTableToNeighbors();

	messages.clear();
	forwardingTableUpdated = false;
}

int main(int argc, const char* argv[])
{
	DistanceVectorNode *dvNode = new DistanceVectorNode();

	dvNode->Initialize(argv[1]);	
	dvNode->GetMyID();
	dvNode->GetMyNeighbors();

	#if logging > 1		
		neighborsIter it = dvNode->neighbors.begin();
		while (it != dvNode->neighbors.end())
		{
			cout << "Connection to: " << it->first << " with cost " << it->second << endl;
			it++;
		}
	#endif

	dvNode->WaitForAllClear();
	dvNode->RequestNeighborConnectionInfo();
	dvNode->SendForwardingTableToNeighbors();
	dvNode->InitializeForwardingTableConnections();
   	dvNode->Listen();

	return 0;
}