#include <iostream>
#include <random>
#include "PlantMonster.h"

using namespace std;

PlantMonster::PlantMonster(const char* scriptname, int n_id)
	:ScriptNpc(scriptname, n_id)
{
	imageType = OBJECT_PLANT;
	L = luaL_newstate();
	luaL_openlibs(L);
	string mScriptname{ "Script/" };
	mScriptname += scriptname;
	int error = luaL_loadfile(L, mScriptname.c_str()) || lua_pcall(L, 0, 0, 0);
	if (error != 0)
	{
		cout << "여기나?" << endl;
		cout << lua_tostring(L, -1) << endl;
	}
	lua_getglobal(L, "set_uid");
	lua_pushnumber(L, n_id);
	if (0 != lua_pcall(L, 1, 0, 0))
	{
		cout << lua_tostring(L, -1) << endl;
		lua_pop(L, 1);
	}

	lua_getglobal(L, "Init");
	lua_pcall(L, 0, 9, 0);
	bool spawntrigger = lua_toboolean(L, -1);
	strcpy_s(name, 20, lua_tostring(L, -2));
	auto spawnAreaY = lua_tointeger(L, -3);
	auto spawnAreaX = lua_tointeger(L, -4);
	level = lua_tointeger(L, -5);
	maxhp = lua_tointeger(L, -6);
	monType = (MonsterType)lua_tointeger(L, -7);
	monMoveType = (MonsterMoveType)lua_tointeger(L, -8);
	Peace_Notice_Range = lua_tointeger(L, -9);
	lua_pop(L, 9);

	if (spawntrigger == false)
	{
		random_device rd;
		mt19937 rng(rd());
		uniform_int_distribution<int> RPos(-990, 990);
		while (1) {
			x = RPos(rng) + spawnAreaX;
			y = RPos(rng) + spawnAreaY;
			if (mMap[x][y] == 0)
				break;
		}

		//추후에 여기에서 바운더 처리 해줘야함. 
		//x = rand() % WORLD_WIDTH;
		//y = rand() % WORLD_HEIGHT;
	}
	else {
		x = spawnAreaX;
		y = spawnAreaY;
	}

	lua_register(L, "API_SendMessage", CPP_SendMessage);
	lua_register(L, "API_get_x", CPP_get_x);
	lua_register(L, "API_get_y", CPP_get_y);
	lua_register(L, "API_AutoMoveNPC", CPP_AutoMoveNPC);
	lua_register(L, "API_get_hp", CPP_get_hp);
	lua_register(L, "API_MonsterDie", CPP_MonsterDie);
	lua_register(L, "API_SendMessageMySight", CPP_SendMessageMySight);
	lua_register(L, "API_get_MaxHp", CPP_get_MaxHp);
	lua_register(L, "API_MonsterAttack", CPP_MonsterAttack);
	lua_register(L, "API_ChaseTarget", CPP_ChaseTarget);
}

PlantMonster::~PlantMonster()
{

}
