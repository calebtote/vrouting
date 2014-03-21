#pragma once
#include "globals.h"
#include "rtngnode.h"

class DistanceVectorNode : public RoutingNode
{
public:
	virtual bool ProcessMessages();

};