#include "manager.h"

RoutingManager::RoutingManager()
{
	activeNodeCount = 0;
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
RoutingManager::Initialize(int myPort)
{
	int length, n;
   	struct sockaddr_in server;
   
  	mySocket = socket(AF_INET, SOCK_DGRAM, 0);
   	if (mySocket < 0) 
   		perror("Opening socket");

   	length = sizeof(server);
   	bzero(&server,length);

   	server.sin_family = AF_INET;
   	server.sin_addr.s_addr = INADDR_ANY;
   	server.sin_port = htons(myPort);

   	if (bind(mySocket,(struct sockaddr *)&server,length) < 0) 
    	perror("binding");
}

int
RoutingManager::WaitForNewNodes()
{
	while(topology.size() > activeNodeCount)
	{
		int n;
		char buffer[1024];
		bzero(buffer,1024);
		unsigned int length = sizeof(struct sockaddr_in);
		struct sockaddr_in from;

		cout << "Waiting for nodes...\n";
		n = recvfrom(mySocket,buffer,1024,0,(struct sockaddr *)&from, &length);
		
		if (n < 0) 
			perror("recvfrom");
		else
			ActivateNewNode(from);
	}
}

bool
RoutingManager::ActivateNewNode(struct sockaddr_in newNode)
{
	TopologyIter iter = topology.begin();
	char ip4[INET_ADDRSTRLEN];

	//check if the topology is set, or if the connection we're trying to activate
	//is already a member of our virtual-network
	if(iter == topology.end() || 
		std::find(activeNodes.begin(), activeNodes.end(), newNode) != activeNodes.end())
			return false;

	do
	{
		if(!iter->second.online)
		{
			char buf[512];		
			GenerateConnectionString(iter->second.id, buf);
			SendMessage(newNode, buf);

			//update our connection data so we can continue talking
			newNode.sin_port = iter->second.id;

			iter->second.online = true;
			iter->second.connection.theirAddress = newNode;
			iter->second.connection.ipstr = inet_ntop(AF_INET, &newNode.sin_addr, ip4, INET_ADDRSTRLEN);
            iter->second.connection.port = iter->second.id;	//listen on port: Virtual-ID

            #if logging > 0
				cout << "Connected Activated!\n";
				cout << "Node ID: " << iter->second.id << endl;
				cout << "Node Port: " << iter->second.connection.port << endl;
			#endif

			activeNodes.push_back(newNode);
			activeNodeCount++;
			
			//let's be terrible developers:
			//wait a bit to make sure a proper connection is setup on the other end
			//500 microseconds - who will notice  :/
			usleep(500);
			SendNeighborInformation(iter->second, buf);

			#if logging > 1
				cout << "Active Nodes: " << activeNodeCount << endl << endl;
			#endif

			return true; //all good
		}

		iter++;

	}while (iter != topology.end());

	return false; //received a connection, but no more nodes to hand out
}

void
RoutingManager::GenerateConnectionString(int id, char* buffer)
{
	bzero(buffer,512);
	sprintf(buffer, "@%d~%d~%d", SERVER_PORT, RoutingMessage::NEWNODE, id);
}

void
RoutingManager::SendNeighborInformation(struct Node n, char* buffer)
{
	NodeNeighborsIter iter = n.neighbors.begin();
	if (iter != n.neighbors.end())
	{
		bzero(buffer,512);
		char dest[5];
		char cost[5];
		snprintf(dest, sizeof(dest), "%d", GenerateVirtualNodeID(iter->first));
		snprintf(cost, sizeof(cost), "%d", iter->second);
		sprintf(buffer, "@%d~%d~%s.%s", SERVER_PORT, RoutingMessage::LINKADD, dest, cost);

		iter++;
		while (iter != n.neighbors.end())
		{	
			snprintf(dest, sizeof(dest), "%d", GenerateVirtualNodeID(iter->first));
			snprintf(cost, sizeof(cost), "%d", iter->second);
			
			char temp[20];
			sprintf(temp, "~%d~%s.%s", RoutingMessage::LINKADD, dest, cost);
			strcat(buffer,temp);
			iter++;
		}

		SendMessage(n.connection.theirAddress, buffer);
	}
}

int
RoutingManager::SendMessage(struct sockaddr_in toNode, char buffer[1024])
{
	#if logging > 1
		cout << "Sending: " << buffer << endl;
	#endif
	
	int n;
	unsigned int length = sizeof(struct sockaddr_in);

	n = sendto(mySocket, buffer, strlen(buffer),0,
				(const struct sockaddr *)&toNode,length);

	if (n < strlen(buffer)) 
		perror("Sendto");

	#if logging > 1
		cout << "Sent: " << n << " bytes of data\n";
	#endif
}

int
RoutingManager::Run()
{
	while(1)
	{
		Listen();
	}
}

int
RoutingManager::Listen()
{
	int n;
	char buffer[512];
	bzero(buffer,512);
	struct sockaddr_in from;
	RoutingMessage parser;

	#if logging > 1
		cout << "Listening...\n";
	#endif

	n = recvfrom(mySocket,buffer,512,0,(struct sockaddr *)&from, &sockLen);
	
	if (n < 0) 
		perror("recvfrom");
	else
	{
		parser.ParseMessage(buffer, fromNode, messages);
		ProcessMessages();
	}
}

int
RoutingManager::ProcessMessages()
{
	messagesIter iter = messages.begin();
	unsigned int length = sizeof(struct sockaddr_in);
	while(iter != messages.end())
	{
		switch(iter->first)
		{
			case RoutingMessage::HELLO: break;
			case RoutingMessage::KEEPALIVE: break;
			case RoutingMessage::MESSAGE: break;			
			case RoutingMessage::REQCONINFO: break;
			case RoutingMessage::ACKCONINFO: break;
			case RoutingMessage::CONVERGING: break;
			case RoutingMessage::REQNBRINFO: break;
			case RoutingMessage::CONVERGED:
			{
				topology.at(fromNode).converged = true;
				cout << fromNode << " has converged!\n";
				break;
			}
			case RoutingMessage::DEBUG: break;
			case RoutingMessage::ERROR: break;
			default: break;
		}
		iter++;
	}

	//all done!
	messages.clear();
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
		n.online = false;
		n.neighbors.insert(pair<int,int>(destID, destCost));

		topology.insert(pair<int, Node>(nodeID, n));

		#if logging > 0
			cout << "\nNode Added: " << n.id << endl;
			cout << "Neighbor {" << destID << "} added to {" << nodeID 
				<< "}, with cost {" << n.neighbors.at(destID) << "}\n\n";
		#endif
	}
	else
	{
		topology.at(nodeID).neighbors.insert(pair<int,int>(destID, destCost));

		#if logging > 0
			cout << "->Neighbor {" << destID << "} added to {" << nodeID 
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
		AddNodeLink(GenerateVirtualNodeID(dest), id, cost);
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
	RoutingManager *manager = new RoutingManager();
	manager->ParseInputFile("topo.txt", 10, 3, " ");
	manager->PopulateTopology();
	cout << "\n\t**** **** ****\n";

	manager->Initialize();
	manager->WaitForNewNodes();

	// send all clear
    TopologyIter topoIter = manager->topology.begin();
	for (; topoIter != manager->topology.end(); topoIter++)
	{
		manager->SendMessage(topoIter->second.connection.theirAddress, "@7777~90~ ~");
	}

	// here we go!
	manager->Run();

	/*
	manager->myConnection = new NetworkConnection(NULL, "5000");
	
	manager->myConnection->SetSocketHints();
	manager->myConnection->PopulateAddressInfo();
	manager->myConnection->BindSocket();
	*/

	cout << "\n\t**** **** ****\n";

	#if logging > 2
		// print the tokens
	    for (int i = 0; i < manager->tokens.size(); i++) // n = # of tokens
	      cout << "Token[" << i << "] = " << manager->tokens[i] << endl;
	    cout << "\n\t**** **** ****\n";
    #endif


	#if logging > 2
		manager->PrintTopology();
		cout << "\n\t**** **** ****\n";
	#endif

	cout << "\n\t****  End.  ****\n";

	return 0;
}/**/
