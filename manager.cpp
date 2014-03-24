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

            #ifdef logging
				cout << "Connected Activated!\n";
				cout << "Node ID: " << iter->second.id << endl;
				cout << "Node Port: " << iter->second.connection.port << endl;
				cout << "Address: " << iter->second.connection.ipstr << endl;
			#endif

			activeNodes.push_back(newNode);
			activeNodeCount++;
			
			//let's be terrible developers:
			//wait a bit to make sure a proper connection is setup on the other end
			//1 second - who will notice  :/
			sleep(1);
			SendNeighborInformation(iter->second, buf);

			#ifdef logging
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
RoutingManager::SendMessageMap(struct Node n)
{
	string theMsg = "";
	nodeMessagesIter iter = nodeMessages.begin();

	while (iter != nodeMessages.end())
	{
		if(iter->first == n.id)
		{
			char *tmp = new char[iter->second.length() + 1];
			strcpy(tmp, iter->second.c_str());
			SendMessage(n.connection.theirAddress, tmp);
		}
		iter++;
	}
}

int
RoutingManager::SendMessage(struct sockaddr_in toNode, char buffer[1024])
{
	#ifdef logging
		cout << "Sending: " << buffer << endl;
	#endif
	
	int n;
	unsigned int length = sizeof(struct sockaddr_in);

	n = sendto(mySocket, buffer, strlen(buffer),0,
				(const struct sockaddr *)&toNode,length);

	if (n < strlen(buffer)) 
		perror("Sendto");

	#ifdef logging
		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &toNode, ip, INET_ADDRSTRLEN);
		cout << "Sent: " << n << " bytes of data to --> " 
			<< ip << ":" << toNode.sin_port << endl;
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
	char buffer[512];
	bzero(buffer,512);
	struct sockaddr_in from;
	string input;
	RoutingMessage parser;

	int inputNode, inputNeighbor, inputCost;

	cout << "Listening...\n";

		/* Temporary fix. 
	/* This should be abstracted, and more widely used
	/********* Start Select() ***********/
	fd_set readfds;
	struct timeval tv;
	tv.tv_sec = 120;
	tv.tv_usec = 0;	//wait for 1.5 second here

	int rv = 1;

	// clear the set ahead of time
	FD_ZERO(&readfds);

	// add our descriptors to the set
	FD_SET(mySocket, &readfds);
	FD_SET(STDIN, &readfds);

	// the n param for select()
	int n = mySocket + 1;
	/********* End Select() ***********/

	while(rv != 0)
	{
		// clear the set ahead of time
		FD_ZERO(&readfds);

		// add our descriptors to the set
		FD_SET(mySocket, &readfds);
		FD_SET(STDIN, &readfds);

		rv = select(n, &readfds, NULL, NULL, &tv);

		if (rv == -1)
			perror("select"); // error occurred in select()

		if (FD_ISSET(STDIN, &readfds))
        {
        		cin >> inputNode >> inputNeighbor >> inputCost;
        		UpdateLink(GenerateVirtualNodeID(inputNode), GenerateVirtualNodeID(inputNeighbor), inputCost);

				/* This is a hack because of time constraints
        		TopologyIter iter = topology.begin();
				while(iter != topology.end())
				{
					iter->second.converged = false;
					iter++;
				}*/

				//everyone is probably converged after 3 seconds
				sleep(3);
				PassMessages();
        }
        else
        {
			bzero(buffer,512);
			int n = recvfrom(mySocket,buffer,512,0,(struct sockaddr *)&from, &sockLen);

			if (n < 0) 
				perror("recvfrom");

			if (n > 0)
			{
				parser.ParseMessage(buffer, fromNode, messages);
				ProcessMessages();

				if(IsNetworkConverged())
				{
					for(TopologyIter it = topology.begin(); it != topology.end(); it++)
						SendMessageMap(it->second);
					
					PassMessages();
				}
			}
		}
	}

//	n = recvfrom(mySocket,buffer,512,0,(struct sockaddr *)&from, &sockLen);
}

int
RoutingManager::UpdateLink(int node, int neighbor, int cost)
{
	char b[512];
	bzero(b,512);
	sprintf(b, "@%d~%d~%d.%d", SERVER_PORT, RoutingMessage::LINKUPDATE, neighbor, cost);
	SendMessage(topology.at(node).connection.theirAddress, b);

	bzero(b,512);
	sprintf(b, "@%d~%d~%d.%d", SERVER_PORT, RoutingMessage::LINKUPDATE, node, cost);
	SendMessage(topology.at(neighbor).connection.theirAddress, b);
}

bool
RoutingManager::IsNetworkConverged()
{
	TopologyIter iter = topology.begin();
	while(iter != topology.end())
	{
		if (!iter->second.converged)
			return false;

		iter++;
	}

	return true;
}

int
RoutingManager::PassMessages()
{
	TopologyIter iter = topology.begin();
	while(iter != topology.end())
	{
		char buffer[512];
		bzero(buffer,512);

		sprintf(buffer, "@%d~%d~ ~", SERVER_PORT, RoutingMessage::PASSMSGS);
		SendMessage(iter->second.connection.theirAddress, buffer);
		iter++;
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
			case RoutingMessage::PASSMSGS: break;
			case RoutingMessage::NEWMESSAGE: break;			
			case RoutingMessage::REQCONINFO: 
			{
				istringstream buffer(iter->second);
				int value;
				buffer >> value;
				string ip = topology.at(value).connection.ipstr;
				cout << "id:" << topology.at(value).id << endl;

				char b[512];
				bzero(b,512);
				sprintf(b, "@%d~%d~%d.%s", SERVER_PORT, RoutingMessage::ACKCONINFO, value, ip.c_str());
				SendMessage(topology.at(fromNode).connection.theirAddress, b);
			}break;
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
		cout << "Topology File Not Found.\n";
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

	theFile.close();

	return 0;
}

int
RoutingManager::ParseMessageFile(char* filePath, const int MAX_CHARS_PER_LINE,
							    const int MAX_TOKENS_PER_LINE, const char* const DELIMITER)
{
	ifstream theFile;
	theFile.open(filePath);
	char buf[MAX_CHARS_PER_LINE];

	if(!theFile.good())
	{
		cout << "Message File Not Found.\n";
		return 1;
	}

	while (theFile.getline(buf, MAX_CHARS_PER_LINE))
	{
		char *temp, *theMsg;
		int from;
		int to;

		temp = strtok(buf, DELIMITER);
		istringstream(temp) >> from;

		temp = strtok(NULL, DELIMITER);
		istringstream(temp) >> to;

		from = GenerateVirtualNodeID(from);
		to = GenerateVirtualNodeID(to);

		theMsg = strtok(NULL, "\n");

		char b[MAX_CHARS_PER_LINE];
		sprintf(b, "@%d~%d~%d.%s", SERVER_PORT, RoutingMessage::NEWMESSAGE, to, theMsg);
		nodeMessages.insert(pair<int,string>(from, b));

		#ifdef logging
			cout << "Message Stored: " << from << "|" << b << endl;
		#endif
	}

	theFile.close();
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

		#ifdef logging
			cout << "\nNode Added: " << n.id << endl;
			cout << "Neighbor {" << destID << "} added to {" << nodeID 
				<< "}, with cost {" << n.neighbors.at(destID) << "}\n\n";
		#endif
	}
	else
	{
		topology.at(nodeID).neighbors.insert(pair<int,int>(destID, destCost));

		#ifdef logging
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
	manager->ParseMessageFile("msg.txt");
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
