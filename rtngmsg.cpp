#include "rtngmsg.h"

bool
RoutingMessage::ParseMessage(char* buffer, int &fromNode, multimap<int, string> &messages, const int MAX_CHARS_PER_LINE, 
							    const int MAX_TOKENS_PER_LINE, const char* const DELIMITER)
{
	#if logging > 1
		cout << "Buffer to parse: " << buffer << endl;
	#endif

    if (buffer[0] != '@')
    {
        perror("Buffer malformated!");
        return false;
    }

	//remove the '@'
	buffer++;

	char buf[MAX_CHARS_PER_LINE];
	strcpy(buf, buffer);
	char* temp = strtok(buf, DELIMITER);
	if (temp == NULL)
	{
		perror("Buffer malformated!");
		return false;
	}

	fromNode = atoi(temp);
	temp = strtok(NULL, DELIMITER);

	vector<string> tokens;
	while(temp != NULL)
	{
		string val(temp);
		tokens.push_back(val);
		temp = strtok(NULL, DELIMITER);
	}

	//store messages in the map: <message-type>, <message>
	for (int i = 0; i < tokens.size(); i+=2)
		messages.insert(pair<int, string>(atoi(tokens[i].c_str()), tokens[i+1]));

	//all good
	return true;
}
