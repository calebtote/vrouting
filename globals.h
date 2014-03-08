#pragma once

// #define logging levels
// these are imaginary levels 
// that sounded nice at the time
// ------------------------------
// 0 : no logging
// 1 : minimal logging
// 2 : verbose logging
#define logging 1

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

bool operator == (const sockaddr_in &lhs, const sockaddr_in& rhs) {
      return lhs.sin_family == rhs.sin_family 
        && lhs.sin_port == rhs.sin_port
        && lhs.sin_addr.s_addr == rhs.sin_addr.s_addr;
       // && lhs.sin_zero == rhs.sin_zero;
}
bool operator != (const sockaddr_in &lhs, const sockaddr_in& rhs) {
      return !(lhs == rhs);
}