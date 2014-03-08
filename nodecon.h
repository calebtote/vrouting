#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
struct NodeConnection
{
	struct sockaddr_in theirAddress; 
	const char* ipstr;
	unsigned short int port;
};