#pragma once
#include <iostream>
#include <string>
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
int AcceptQuest(const char* name, int q_code);
int ChangeQuestProperty(const char* name, int q_code, int q_progress);

class PrepareStatement {
	MYSQL* mMysql;
	MYSQL_STMT* mPstmt;
	int mAffectRowCnt;
	std::string mQuery;
	char mQueryType;
public:
	PrepareStatement(MYSQL* sql);
	~PrepareStatement();
	void SetQuery(const char* query);
	void AddDataParameter(MYSQL_BIND* params);
	void AddResultParameter(MYSQL_BIND* params);
	void Excute();
	int Fetch();
	int GetResponse();
	void ReadyToResponse();
private:
	void SetQueryType();
	int GetOneResponse();
};