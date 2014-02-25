#include "tcpcon.h"

const string TCPConnection::Client = "client";
const string TCPConnection::Server = "server";

TCPConnection::TCPConnection(const char* t, const char* p) : target(t), port(p)
{ }

void 
TCPConnection::sigchld_handler()
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *
TCPConnection::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void
TCPConnection::SetHints()
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
TCPConnection::PopulateAddressInfo()
{
	int retVal =  getaddrinfo(GetTarget(), GetPort(), &hints, &serverInfo);
	if (retVal != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retVal));
        return 1;
    }

    return 0;
}

int
TCPConnection::BindSocket()
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


/*
int tcp_connect(const char *serv, const char *host = NULL)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(host, serv, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
        	StartMasterMind(new_fd,their_addr);
        }
       close(new_fd);  // parent doesn't need this
    }

    return 0;
}*/