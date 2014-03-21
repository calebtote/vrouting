#pragma once
#include "globals.h"


const int MAX_CHARS_PER_LINE = 512;
/*	Message Structures
		-- Routing messages must begin with the literal token "~"
		-- Verified messages will be acknowledged (maybe.. why wouldn't I just use tcp)
		-- If we receive an incomplete message, we discard

		Messages must be in the following format:
			@nnnn~xx~(~message)
				- nnnn : virtual node-id
				- xx :  message type				
				- message : (optional) string message
			Example:  @3702~01<eom>
				- Says: Hello, I'm router 3702
			Example:  @3702~10~Howdy
				- Says: I'm 3702 sending you the message: Howdy
			Example:  @3702~01~ ~10~Howdy
				- Says: Hello, I'm router 3702. I'm sending you the message: Howdy
*/

class RoutingMessage
{
public:
	bool ParseMessage(char* buffer, int &fromNode, multimap<int, string> &messages, const int MAX_CHARS_PER_LINE = 512, 
							    const int MAX_TOKENS_PER_LINE = 20, const char* const DELIMITER = "~");


	//defined message types
	//multi-token strings must have a space between each delimeter
	//regardless of whether or not it's used (i.e., '~ ~')
	enum
	{
		HELLO = 1, //@7777~01~ ~10~hi
		KEEPALIVE = 0,
		PASSMSGS = 10, //Send trigger for message passing
		NEWMESSAGE = 11, //@7777~11~3701.Send this message to node-3701 please!
		FWDMESSAGE = 12, //@3703~12~3701.Send this message to node-3701 please!

		NEWNODE = 15, //@7777~15~3701 :: Your Node ID is 3701
		REQCONINFO = 16, //@3701~16~3702 :: Give me node 3702 connection info
		ACKCONINFO = 17, //@7777~17~3702.hostname.port
		LSP = 18,
		CONVERGING = 20,
		LINKADD = 21, //@7777~21~3702.11 :: Connected to 3702 with cost 11
		LINKUPDATE = 22, //same as above, just with code 22
		VECTOR = 23,
		REQNBRINFO = 25, //@7777~25~3701 :: Give me my neighbors
		CONVERGED = 30,

		ALLCLEAR = 90,	//@7777~90~ ~ :: All nodes connected - start converging
		ERROR = 99,
		DEBUG = 98
	};

private:
	char buf[MAX_CHARS_PER_LINE];
};