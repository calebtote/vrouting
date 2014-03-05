#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
struct NodeConnection
{
	int fd;
	socklen_t sin_size;
	struct sockaddr_storage theirAddress; 
	char ipstr[INET6_ADDRSTRLEN];
	int port;
};