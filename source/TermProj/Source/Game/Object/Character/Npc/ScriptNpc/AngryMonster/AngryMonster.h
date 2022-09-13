#pragma once
#include "../ScriptNpc.h"
class AngryMonster :public ScriptNpc
{
public:
	AngryMonster(const char* scriptname, int n_id);
	virtual ~AngryMonster();


	int AgroRange;
};

