#pragma once
#include "../Character.h"
#include "../../../../../Script/cplustolua.h"
#pragma comment(lib,"lua54.lib")
#define RANDOMMOVE 5
#define INVALID_TARGET -1
enum MonsterType{Peace,Agro};
enum MonsterMoveType{MT_Siege,MT_Move};
class Npc : public Character
{
public:
	Npc(TYPE type, 
		MonsterType montype = MonsterType::Peace,
		MonsterMoveType monmovetype = MonsterMoveType::MT_Siege
	);
	virtual ~Npc();

	lua_State* L;
	std::mutex lua_lock;
	std::atomic_bool isMoving;
	MonsterType monType;
	MonsterMoveType monMoveType;
	std::chrono::system_clock::time_point attack_cooltime;
	int target;	//몬스터 어글자 대상
	int attackRange;
};

