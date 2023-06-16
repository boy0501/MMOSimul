#pragma once
#include "../Character.h"
#include "../../../../../Script/cplustolua.h"
#pragma comment(lib,"lua54.lib")
class Npc : public Character
{
public:
	Npc(TYPE type
	);
	virtual ~Npc();

	std::atomic_bool isMoving;
};

