#include <iostream>
#include <unordered_set>
#include "ScriptNpc.h"
#include "../../../../Network/Network.h"
#include "../../Player/Player.h"

using namespace std;


ScriptNpc::ScriptNpc(const char* scriptname,int n_id)
	:Npc(TYPE::SCRIPTNPC)
	, FailToChaseTarget(0)
	, Peace_Notice_Range(0)
	,isMoving(false)
{

}

ScriptNpc::~ScriptNpc()
{

}
