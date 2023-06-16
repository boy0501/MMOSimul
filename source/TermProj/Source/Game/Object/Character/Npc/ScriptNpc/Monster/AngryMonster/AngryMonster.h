#pragma once
#include "../Monster.h"
class AngryMonster :public Monster
{
public:
	AngryMonster(const char* scriptname, int n_id);
	virtual ~AngryMonster();


	int AgroRange;
};

