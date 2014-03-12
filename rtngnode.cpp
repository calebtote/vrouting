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
		if (n > 0) cout << buffer << endl;
	}

	parser.ParseMessage(buffer, fromNode, messages);
	ProcessMessages();

	if (n < 0) 
		perror("recvfrom");
}

int
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
			case RoutingMessage::MESSAGE: break;
			case RoutingMessage::NEWNODE: 
				{
					myID = atoi(iter->second.c_str());
					BindSocketToPort();
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

					#if logging > 1
						cout << "Adding " << destID << "|" << destCost << endl;
					#endif

					neighbors.insert(pair<int,int>(destID, destCost));
				} break;
			case RoutingMessage::LINKUPDATE: break;
			case RoutingMessage::REQNBRINFO: break;
			case RoutingMessage::CONVERGED: break;
			case RoutingMessage::DEBUG: break;
			case RoutingMessage::ERROR: break;
			default: break;
		}
		iter++;
	}

	//all done!
	messages.clear();
}

/**/
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

    /*
	rnod->myConnection = new NetworkConnection("localhost", "5000");

	rnod->myConnection->SetSocketHints();
	rnod->myConnection->PopulateAddressInfo();
	rnod->myConnection->BindSocket();

	*/

	return 0;
}/**/
