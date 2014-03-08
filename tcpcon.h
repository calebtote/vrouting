/*******************************************************************/
/*******************************************************************/
/********         THIS FILE NOT CURRENTLY USED             *********/
/*******************************************************************/
/*******************************************************************/


#pragma once

#include "globals.h"
#include "nodecon.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

class NetworkConnection{

	public:

		NetworkConnection(const char *target, const char *port);

		/*************************/
		/* Accessors & Setters   */
		/*************************/
		struct addrinfo GetHints() const
	    { return hints; }
	    void setHints(struct addrinfo hints)
	    { this->hints = hints; }

	    struct addrinfo* GetP() const
	    { return p; }
	    void setP(struct addrinfo* p)
	    { this->p = p; }

	 /* int GetRv() const
	    { return rv; }
	    void setRv(int rv)
	    { this->rv = rv; }*/

	    char GetRemoteAddress() const
	    { return remoteAddress; }
	    void setS(char remoteAddress)
	    { this->remoteAddress = remoteAddress; }

	    struct addrinfo* GetServerInfo() const
	    { return serverInfo; }
	    void setServerInfo(struct addrinfo* serverInfo)
	    { this->serverInfo = serverInfo; }

	    struct sigaction GetSigAction() const
	    { return sigAction; }
	    void setSigAction(struct sigaction sigAction)
	    { this->sigAction = sigAction; }

	    struct sockaddr_storage GetTheirAddress() const
	    { return theirAddress; }
	    void setTheirAddress(struct sockaddr_storage theirAddress)
	    { this->theirAddress = theirAddress; }

	    socklen_t GetSinSize() const
	    { return sin_size; }
	    void setSinSize(socklen_t sinSize)
	    {  sin_size = sinSize; }

	    int GetSocketFd() const
	    { return socketFd; }
	    void setSocketFd(int socketFd)
	    { this->socketFd = socketFd; }

	    const char * GetTarget() const
	    { return target; }
	    void setTarGet(char *target)
	    { this->target = target; }

	    const char * GetPort() const
	    { return port; }
	    void setPort(char *port)
	    { this->port = port; }

	  int GetYes() const
	    { return yes; }
	    void setYes(int yes)
	    { this->yes = yes; }

		/******************************/
		/* End: Accessors & Setters   */
		/******************************/
	    void *get_in_addr(struct sockaddr *sa);
	    void SetSocketHints(int socketType = SOCK_DGRAM);
	    int PopulateAddressInfo();
	    int BindSocket();

	    //udp
	    int GetData();
	    int SendData();

	    //tcp
	    int ListenForConnections();
	    int ConnectToHost();

		vector<NodeConnection> newConnections;

	public:
		

		//if target is NULL, we are hosting
		const char *target;
		const char *port;

		int socketFd;
		struct addrinfo hints;
		struct addrinfo *serverInfo;
		struct addrinfo *p;
		struct sockaddr_storage theirAddress; 

		socklen_t sin_size;
		struct sigaction sigAction;

		char remoteAddress;

		int yes; // for unix

		static const string Client;
		static const string Server;
};
