#pragma once
#include "../Monster.h"
class NormalMonster : public Monster
{
public:
	NormalMonster(const char* scriptname, int n_id);
	virtual ~NormalMonster();

};

