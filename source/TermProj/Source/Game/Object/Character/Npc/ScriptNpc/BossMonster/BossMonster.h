#pragma once
#include "../ScriptNpc.h"
class BossMonster :public ScriptNpc
{
public:
	BossMonster(const char* scriptname, int n_id);
	virtual ~BossMonster();


	int AgroRange;
};

