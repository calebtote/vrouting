#include "tcpcon.h"

const string NetworkConnection::Client = "client";
const string NetworkConnection::Server = "server";

//basic message structure
struct msgstruct {
        int length;
        char* send_data;
};

//basic method for sending messages
int sendMsg(int client, char* theMsg)
{
        msgstruct message;
        message.send_data = theMsg;
        message.length = strlen(message.send_data);

        return (send(client, message.send_data, message.length, 0));
}

NetworkConnection::NetworkConnection(const char* t, const char* p) : target(t), port(p)
{ }

// get sockaddr, IPv4 or IPv6:
void *
NetworkConnection::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void
NetworkConnection::SetSocketHints()
{
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    //this flag is ignored if this->target != NULL
    hints.ai_flags = AI_PASSIVE;

    #ifdef logging
        cout << "Hints set.\n";
    #endif
}

int
NetworkConnection::PopulateAddressInfo()
{
    int retVal =  getaddrinfo(GetTarget(), GetPort(), &hints, &serverInfo);
    if (retVal != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retVal));
        return 1;
    }

    return 0;
}

int
NetworkConnection::BindSocket()
{
    #if logging > 0
        cout << "Binding Socket...";
    #endif

    struct addrinfo *temp = GetServerInfo();

    // loop through all the results and bind to the first we can
    for(; temp != NULL; temp = temp->ai_next) 
    {
        socketFd = socket(temp->ai_family, 
                            temp->ai_socktype, 
                            temp->ai_protocol);

        if (socketFd == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(socketFd, temp->ai_addr, temp->ai_addrlen) == -1) {
            close(socketFd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (temp == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    freeaddrinfo(GetServerInfo());

    #if logging > 0
        cout << " Socket Bound!\n";
    #endif

    return 0;
}



int
NetworkConnection::ListenForConnections()
{
    char s[INET6_ADDRSTRLEN];
    fcntl(socketFd, F_SETFL, O_NONBLOCK);

    if (listen(socketFd, 15) == -1) {
        perror("listen");
        exit(1);
    }

//    sigAction.sa_handler = sigchld_handler; // reap all dead processes    
//    sigemptyset(&sigAction.sa_mask);
//    sigAction.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sigAction, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

 //   while(1) {  // main accept() loop
        sin_size = sizeof theirAddress;
        int new_fd = accept(socketFd, (struct sockaddr *)&theirAddress, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            return 1;
        }

        inet_ntop(theirAddress.ss_family,
            get_in_addr((struct sockaddr *)&theirAddress),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

      //  if (!fork()) { // this is the child process
       //     close(socketFd); // child doesn't need the listener

            NodeConnection nc = NodeConnection();

            char ipstr[INET6_ADDRSTRLEN];
            getpeername(new_fd, (struct sockaddr*)&theirAddress, &sin_size);
            struct sockaddr_in *soc = (struct sockaddr_in *)&theirAddress;
            int port = ntohs(soc->sin_port);
            inet_ntop(AF_INET, &soc->sin_addr, nc.ipstr, sizeof ipstr);
            
            nc.fd = new_fd;
            nc.theirAddress = sockaddr_storage(theirAddress);
            nc.sin_size = sin_size;
            nc.port = port;
            newConnections.push_back(nc);
      //      exit(0);
       // }
      //  close(new_fd);  // parent doesn't need this
  //  }
}
