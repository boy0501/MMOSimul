#include <iostream>
#include <unordered_set>
#include "ScriptNpc.h"
#include "../../../../Network/Network.h"
#include "../../Player/Player.h"

using namespace std;


void ScriptNpc::SpawnNPC()
{
	lua_getglobal(L, "set_SpawnXY");
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	if (0 != lua_pcall(L, 2, 0, 0))
	{
		cout << lua_tostring(L, -1) << endl;
		lua_pop(L, 1);
	}
}

ScriptNpc::ScriptNpc(const char* scriptname,int n_id)
	:Npc(TYPE::SCRIPTNPC)
	,isMoving(false)
	, L(nullptr)
{

}

ScriptNpc::~ScriptNpc()
{

}
