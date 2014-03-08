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

/**/
int main(int argc, const char* argv[])
{
	RoutingNode *rnod = new RoutingNode();
	rnod->Initialize("localhost");

	//remove this crap when stuff gets more reliable
	fcntl(rnod->mySocket, F_SETFL, O_NONBLOCK);

    /*
	rnod->myConnection = new NetworkConnection("localhost", "5000");

	rnod->myConnection->SetSocketHints();
	rnod->myConnection->PopulateAddressInfo();
	rnod->myConnection->BindSocket();

	*/
	while(1)
	{
		int n;
		char buffer[256];
		bzero(buffer,256);
		unsigned int length = sizeof(struct sockaddr_in);
		struct sockaddr_in from;
   
		n = sendto(rnod->mySocket,"HELLO!!", strlen("HELLO!!"),0,(const struct sockaddr *)&rnod->server,length);

		if (n < 0) 
			perror("Sendto");

		n = recvfrom(rnod->mySocket,buffer,256,0,(struct sockaddr *)&from, &length);
		
		if (n < 0) 
			perror("recvfrom");

   		cout << "Got ack:" << buffer << endl;
	}

	return 0;
}/**/
