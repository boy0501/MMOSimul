#pragma once

extern "C" {
#include "include/lua.h"
#include "include/lauxlib.h"
#include "include/lualib.h"
}

int CPP_SendMessage(lua_State* L);
int CPP_get_x(lua_State* L);
int CPP_get_y(lua_State* L);
int CPP_AutoMoveNPC(lua_State* L);
int CPP_get_hp(lua_State* L);
int CPP_get_MaxHp(lua_State* L);
int CPP_MonsterDie(lua_State* L);
int CPP_SendMessageMySight(lua_State* L);
int CPP_MonsterAttack(lua_State* L);
int CPP_ChaseTarget(lua_State* L);
int CPP_TelePortTarget(lua_State* L);
int CPP_BossDeBuff(lua_State* L);
int CPP_BossBuffMySight(lua_State* L);
int CPP_NoticeWindow(lua_State* L);
int CPP_NoticeWindowOK(lua_State* L);
int CPP_SetCoroutine(lua_State* L);
int CPP_GetCoroutine(lua_State* L);