#include "Npc.h"


Npc::Npc(TYPE type,	MonsterType montype ,MonsterMoveType monmovetype)
	:Character(type)
	,monType(montype)
	,monMoveType(monmovetype)
	,attack_cooltime(std::chrono::system_clock::now())
	,isMoving(false)
	,target(INVALID_TARGET)
	,attackRange(0)
	,L(nullptr)
{

}

Npc::~Npc()
{

}
