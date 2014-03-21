#include "distvec.h"

bool
DistanceVectorNode::ProcessMessages()
{
	if(RoutingNode::ProcessMessages())
		return true;

	messagesIter iter = messages.begin();
	unsigned int length = sizeof(struct sockaddr_in);
	while(iter != messages.end())
	{
		switch(iter->first)
		{
		}
	}

	if(forwardingTableUpdated)
		SendForwardingTableToNeighbors();

	messages.clear();
	forwardingTableUpdated = false;
}

int main(int argc, const char* argv[])
{
	DistanceVectorNode *dvNode = new DistanceVectorNode();

	dvNode->Initialize("localhost");	
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
	dvNode->SendForwardingTableToNeighbors();
	dvNode->InitializeForwardingTableConnections();
   	dvNode->Listen();

	return 0;
}