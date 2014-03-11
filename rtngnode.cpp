#include "rtngnode.h"

int
RoutingNode::Initialize(const char* theManager)
{
	int n;
   	unsigned int length;
   	struct hostent *hp;
   	char buffer[256];
   
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
   	neighbor.sin_port = htons(myID);

   	if (bind(neighborSocket,(struct sockaddr *)&neighbor,length) < 0) 
    	perror("binding");
}

int
RoutingNode::GetMyID()
{
	int n;
	char buffer[256];
	bzero(buffer,256);
	unsigned int length = sizeof(struct sockaddr_in);
	struct sockaddr_in from;

	n = sendto(mySocket,"!", strlen("!"),0,(const struct sockaddr *)&server,length);

	if (n < 0) 
		perror("Sendto");

	n = recvfrom(mySocket,buffer,256,0,(struct sockaddr *)&from, &length);

	if (n < 0) 
		perror("recvfrom");

	parser.ParseMessage(buffer, fromNode, messages);
	ProcessMessages();


	//once we have our information from the manager, let's hog some cpu
	//remove this crap when stuff gets more reliable
	fcntl(mySocket, F_SETFL, O_NONBLOCK);
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
			case RoutingMessage::NEWNODE: break;
				myID = atoi(iter->second.c_str());
				CreateNeighborSocket();
				sendto(neighborSocket,"!", strlen("!"),0,(const struct sockaddr *)&server,length);
				break;			
			case RoutingMessage::REQCONINFO: break;
			case RoutingMessage::ACKCONINFO: break;
			case RoutingMessage::CONVERGING: break;
			case RoutingMessage::LINKADD: break;
			case RoutingMessage::LINKUPDATE: break;
			case RoutingMessage::CONVERGED: break;
			case RoutingMessage::DEBUG: break;
			case RoutingMessage::ERROR: break;
			default: break;
		}
	}
}

/**/
int main(int argc, const char* argv[])
{
	RoutingNode *rnod = new RoutingNode();
	rnod->Initialize("localhost");	
	rnod->GetMyID();

	cout << "Node: " << rnod->fromNode << endl;

	messagesIter iter = rnod->messages.begin();
	for (iter; iter != rnod->messages.end(); iter++)
		cout << "Message Type: " << iter->first << " Message: " << iter->second << endl;

    /*
	rnod->myConnection = new NetworkConnection("localhost", "5000");

	rnod->myConnection->SetSocketHints();
	rnod->myConnection->PopulateAddressInfo();
	rnod->myConnection->BindSocket();

	*/

	return 0;
}/**/
