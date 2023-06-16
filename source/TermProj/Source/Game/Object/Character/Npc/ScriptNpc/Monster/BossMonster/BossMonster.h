#pragma once
#include "../Monster.h"
class BossMonster :public Monster
{
public:
	BossMonster(const char* scriptname, int n_id);
	virtual ~BossMonster();


	int AgroRange;
};

