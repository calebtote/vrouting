#pragma once

// #define logging levels
// these are imaginary levels 
// that sounded nice at the time
// ------------------------------
// 0 : no logging
// 1 : informational
// 2 : debugging
// 3 : give me er'thang
#define logging 0

#define STDIN 0

// standard includes because I'm lazy
// this is so bad...
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <string.h>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <map>

#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>

using namespace std;

typedef map<int,string, less<int> >::iterator messagesIter;

const int SERVER_PORT = 7777;