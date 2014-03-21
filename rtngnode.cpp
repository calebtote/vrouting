#include "rtngnode.h"

int
RoutingNode::Initialize(const char* theManager)
{
	int n;
   	unsigned int length;
   	struct hostent *hp;
   	char buffer[512];
   
   	mySocket = socket(AF_INET, SOCK_DGRAM, 0);
   	if (mySocket < 0) 
   		perror("socket");

   	server.sin_family = AF_INET;
   	hp = gethostbyname(theManager);
   	if (hp==0) 
   		perror("Unknown host");

   	bcopy((char *)hp->h_addr, 
        (char *)&server.sin_addr,
         hp->h_length);

   	server.sin_port = htons(7777);
}

int
RoutingNode::SendMessage(struct sockaddr_in toNode, char buffer[512])
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
RoutingNode::BindSocketToPort()
{
	mySocket = socket(AF_INET, SOCK_DGRAM, 0);
	   	if (mySocket < 0)
	   		perror("socket");
	struct sockaddr_in service;
	int length = sizeof(service);
   	bzero(&service,length);

   	service.sin_family = AF_INET;
   	service.sin_addr.s_addr = INADDR_ANY;
   	service.sin_port = myID;

   	if (bind(mySocket,(struct sockaddr *)&service,length) < 0)
   	{
    	perror("binding");
    	return -1;
   	}

    #if logging > 1
    	cout << "Socket Bound: " << mySocket << ":" << service.sin_port << endl;
    #endif
}

int
RoutingNode::CreateNeighborSocket()
{
	int length, n;
   
  	neighborSocket = socket(AF_INET, SOCK_DGRAM, 0);
   	if (neighborSocket < 0) 
   		perror("Opening socket");

   	length = sizeof(neighbor);
   	bzero(&neighbor,length);

   	neighbor.sin_family = AF_INET;
   	neighbor.sin_addr.s_addr = INADDR_ANY;
   	neighbor.sin_port = myID;

   	if (bind(neighborSocket,(struct sockaddr *)&neighbor,length) < 0) 
    	perror("binding");

    #if logging > 1
    	cout << "Socket Created: " << neighborSocket << ":" << neighbor.sin_port << endl;
    #endif
}

int
RoutingNode::GetMyID()
{
	int n;
	char buffer[512];
	bzero(buffer,512);
	unsigned int length = sizeof(struct sockaddr_in);
	struct sockaddr_in from;

	n = sendto(mySocket,"!", strlen("!"),0,(const struct sockaddr *)&server,length);

	if (n < 0) 
		perror("Sendto");

	n = recvfrom(mySocket,buffer,512,0,(struct sockaddr *)&from, &length);

	if (n < 0) 
		perror("recvfrom");

	parser.ParseMessage(buffer, fromNode, messages);
	ProcessMessages();

	//once we have our information from the manager, let's hog some cpu
	//remove this crap when stuff gets more reliable
	//fcntl(mySocket, F_SETFL, O_NONBLOCK);
}

int
RoutingNode::GetMyNeighbors()
{
	unsigned int length = sizeof(neighbor);
	char buffer[512];
	bzero(buffer,512);
	int n;

	//once we have our information from the manager, let's hog some cpu
	//remove this crap when stuff gets more reliable
	fcntl(mySocket, F_SETFL, O_NONBLOCK);

	//replace with message-type parser at some point
	while (buffer[6] != '2' && buffer[7] != '1')
	{
		n = recvfrom(mySocket,buffer,512,0,(struct sockaddr *)&server, &length);
		//commented out for code submission
		//if (n > 0) cout << buffer << endl;
	}

	fcntl(mySocket, F_SETFL, ~O_NONBLOCK);

	parser.ParseMessage(buffer, fromNode, messages);
	ProcessMessages();

	if (n < 0) 
		perror("recvfrom");
}

//TODO: Seriously need some better tokenizing
bool
RoutingNode::ProcessMessages()
{
	messagesIter iter = messages.begin();
	unsigned int length = sizeof(struct sockaddr_in);
	while(iter != messages.end())
	{
		switch(iter->first)
		{
			case RoutingMessage::HELLO: break;
			case RoutingMessage::KEEPALIVE: break;
			case RoutingMessage::PASSMSGS: 
			{
				messagesToSendIter it = messagesToSend.begin();
				while(it != messagesToSend.end())
				{
					neighbor.sin_port = forwardingTable.at(it->first).begin()->first;
					char *cstr = new char[it->second.length() + 1];
					strcpy(cstr, it->second.c_str());
					char buf[512];
					sprintf(buf, "@%d~%d~%d.from %d to %d hops %d.%s", 
								myID, RoutingMessage::FWDMESSAGE, it->first, myID-3700, 
								(it->first)-3700, myID-3700, cstr);
					SendMessage(neighbor, buf);
					it++;
				}
				break;
			}
			case RoutingMessage::NEWMESSAGE:
			{
				int destID;
				string message;
				string buf(iter->second);
				string delim = ".";
				destID = atoi(buf.substr(0, buf.find(delim)).c_str());
				buf.erase(0, buf.find(delim) + delim.length());
				message = buf.substr(0, buf.find(delim));
				messagesToSend.insert(pair<int,string>(destID, message));

				#if logging > 1
					cout << "Message Added: " << destID << "." << message<< endl;
				#endif
				break;
			}
			case RoutingMessage::FWDMESSAGE:
			{
				//Message Format:
				//@FromNode~12~DestNode.from x to y hops .<the message>
				int destID;
				string changableStr, finalMessage;
				string buf(iter->second);
				string delim = ".";
				destID = atoi(buf.substr(0, buf.find(delim)).c_str());
				buf.erase(0, buf.find(delim) + delim.length());

				changableStr = buf.substr(0, buf.find(delim));

				stringstream sstm;
				sstm << changableStr << " " << myID-3700;
				changableStr = sstm.str();
				buf.erase(0, buf.find(delim) + delim.length());

				string theMsg = buf.substr(0, buf.find(delim));

				stringstream sstm2;
				sstm2 << changableStr << "." << theMsg;
				finalMessage = sstm2.str();				
	
				if(destID != myID)
				{
					neighbor.sin_port = forwardingTable.at(destID).begin()->first;				
					char *cstr = new char[finalMessage.length() + 1];
					strcpy(cstr, finalMessage.c_str());
					char buf[512];
					sprintf(buf, "@%d~%d~%d.%s", myID, RoutingMessage::FWDMESSAGE, destID, cstr);
					SendMessage(neighbor, buf);
				}

				PrintMessage(finalMessage);
				break;
			}
			case RoutingMessage::NEWNODE: 
			{
				myID = atoi(iter->second.c_str());
				BindSocketToPort();
				UpdateForwardingTable(myID, myID, 0);
			} break;			
			case RoutingMessage::REQCONINFO: break;
			case RoutingMessage::ACKCONINFO: break;
			case RoutingMessage::CONVERGING: break;			
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
				UpdateForwardingTable(destID, destID, destCost);
			} break;
			case RoutingMessage::LINKUPDATE: break;
			case RoutingMessage::VECTOR:
			{
				int destID, destCost;
				string buf(iter->second);
				string delim = ".";	
				destID = atoi(buf.substr(0, buf.find(delim)).c_str());
				buf.erase(0, buf.find(delim) + delim.length());
				destCost = atoi(buf.substr(0, buf.find(delim)).c_str());

				if( IsNewNode(destID) ||
					IsBetterPath(destID, destCost + GetNeighborLinkCost(fromNode)) )
				{
					UpdateForwardingTable(destID, fromNode, destCost + GetNeighborLinkCost(fromNode));
				}
			}
			case RoutingMessage::REQNBRINFO: break;
			case RoutingMessage::CONVERGED: break;
			case RoutingMessage::DEBUG: break;
			case RoutingMessage::ERROR: break;
			default: break;
		}
		iter++;
	}

	//TEMPORARY - FIX THIS
	return false;
}

int
RoutingNode::PrintMessage(string msg)
{
	string buf(msg);
	string delim = ".";
	string header = buf.substr(0, buf.find(delim));
	buf.erase(0, buf.find(delim) + delim.length());
	string footer = buf.substr(0, buf.find(delim));

	cout << header << " message " << footer << endl;
}

int
RoutingNode::AppendMyID(string &theMessage)
{

}

bool
RoutingNode::IsNewNode(int id)
{ return forwardingTable.find(id) == forwardingTable.end(); }

int
RoutingNode::GetNeighborLinkCost(int id)
{ return neighbors.at(id); }

bool
RoutingNode::IsBetterPath(int destID, int destCost)
{ return forwardingTable.at(destID).begin()->second > destCost; }

int
RoutingNode::UpdateForwardingTable(int dest, int hop, int cost)
{
	if(!IsNewNode(dest))
		forwardingTable.erase(dest);

	forwardingTable[dest][hop] = cost;
	//cin.get();

	forwardingTableIter iter = forwardingTable.begin();	
	#if logging > 1
		cout << "\t\t\t*** Forwarding Table Updated! ***\n";
		while(iter != forwardingTable.end())
		{
			map<int,int, less<int> >::iterator inner_it = iter->second.begin();
			cout << "\t\t\tDest: " << iter->first << "\tHop: " << inner_it->first << "\tCost: " << inner_it->second << endl << "   ***   \n";
			iter++;
		}
	#else
		while(iter != forwardingTable.end())
		{
			map<int,int, less<int> >::iterator inner_it = iter->second.begin();
			cout << iter->first << " " << inner_it->first << " " << inner_it->second << endl;
			iter++;
		}
		cout << endl << endl;
	#endif

	forwardingTableUpdated = true;
}

int
RoutingNode::SendForwardingTableToNeighbors()
{
	forwardingTableIter iter = forwardingTable.begin();
	map<int,int, less<int> >::iterator inner_it = iter->second.begin();
	if (iter != forwardingTable.end())
	{
		char buffer[512];
		bzero(buffer,512);
		char dest[5];
		char hop[5];
		char cost[5];
		snprintf(dest, sizeof(dest), "%d", iter->first);
		snprintf(cost, sizeof(cost), "%d", inner_it->second);
		sprintf(buffer, "@%d~%d~%s.%s", myID, RoutingMessage::VECTOR, dest, cost);

		iter++;
		while (iter != forwardingTable.end())
		{	
			inner_it = iter->second.begin();
			snprintf(dest, sizeof(dest), "%d", iter->first);
			snprintf(cost, sizeof(cost), "%d", inner_it->second);
			
			char temp[20];
			sprintf(temp, "~%d~%s.%s", RoutingMessage::VECTOR, dest, cost);
			strcat(buffer,temp);
			iter++;
		}

		Broadcast(buffer);
	}
}

int
RoutingNode::Broadcast(char* buffer)
{
	neighborsIter nbrIter = neighbors.begin();
	while(nbrIter != neighbors.end())
	{
		#if logging > 1
			cout << "Broadcasting!\n";
		#endif

		neighbor.sin_port = nbrIter->first;
		SendMessage(neighbor, buffer);
		nbrIter++;
	}
}

int
RoutingNode::WaitForAllClear()
{
	char buffer[512];
	bzero(buffer,512);

	//replace with message-type parser at some point
	while (buffer[6] != '9' && buffer[7] != '0')
	{
		int n = recvfrom(mySocket,buffer,512,0,(struct sockaddr *)&server, &sockLen);
		/*commented out for code submission
		if (n > 0)
		{ cout << buffer << endl; }*/
	}
}

int
RoutingNode::InitializeForwardingTableConnections()
{
	char buffer[512];

	/* Temporary fix. 
	/* This should be abstracted, and more widely used
	/********* Start Select() ***********/
	fd_set readfds;
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 500000;	//wait for 1.5 second here

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

	char converged[20];
	sprintf(converged, "@%d~%d~ ~", myID, RoutingMessage::CONVERGED);
	SendMessage(server, converged);

	return 0;
}

int
RoutingNode::Listen()
{
	char buffer[512];
	#if logging > 1
		cout << "\n\n\t\t*** Listening... ***\n\n";
	#endif

	while(1)
	{
		bzero(buffer,512);
		int n = recvfrom(mySocket,buffer,512,0,(struct sockaddr *)&neighbor, &sockLen);
		if (n > 0)
		{
			parser.ParseMessage(buffer, fromNode, messages);
			ProcessMessages();
		}
	}
}

/*
int main(int argc, const char* argv[])
{
	RoutingNode *rnod = new RoutingNode();
	rnod->Initialize("localhost");	
	rnod->GetMyID();
	rnod->GetMyNeighbors();

	#if logging > 1		
		neighborsIter it = rnod->neighbors.begin();
		while (it != rnod->neighbors.end())
		{
			cout << "Connection to: " << it->first << " with cost " << it->second << endl;
			it++;
		}
	#endif

	rnod->WaitForAllClear();
	rnod->SendForwardingTableToNeighbors();
	rnod->InitializeForwardingTableConnections();
   	rnod->Listen();

	return 0;
}/**/
