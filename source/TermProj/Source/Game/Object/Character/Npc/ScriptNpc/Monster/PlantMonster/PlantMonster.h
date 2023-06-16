#pragma once
#include "../Monster.h"
class PlantMonster : public Monster
{
public:
	PlantMonster(const char* scriptname, int n_id);
	virtual ~PlantMonster();

};

