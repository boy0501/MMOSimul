#pragma once
#include "../Npc.h"
class ScriptNpc : public Npc
{

public:
	ScriptNpc(const char* scriptname,int n_id);
	virtual ~ScriptNpc();

	void SpawnNPC();
	std::atomic<int> isMoving;
	lua_State* L;
	std::mutex lua_lock;
};

