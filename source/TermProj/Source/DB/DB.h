#pragma once
#include <iostream>
#include "../Game/Network/Network.h"
#include <mysql.h>
#include <atlconv.h>


extern MYSQL* hmysql;

void mysqlTestCode();
void InitializeDB();
void ReleaseDB();
int Login(const char* name, const char* pw, LoginInfo& p_info);
int MakeCharacterAndLogin(const char* name,const char* pw, LoginInfo& p_info);
void SavePos(const char* name, int x, int y);
void SaveStatus(const char* name, short hp, short maxhp, short level, short exp,short x, short y);