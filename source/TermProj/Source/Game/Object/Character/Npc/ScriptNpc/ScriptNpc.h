#pragma once
#include "../Npc.h"
class ScriptNpc : public Npc
{
public:
	ScriptNpc(const char* scriptname,int n_id);
	virtual ~ScriptNpc();

	int FailToChaseTarget;
	std::atomic<int> isMoving;
	int Peace_Notice_Range;
};

