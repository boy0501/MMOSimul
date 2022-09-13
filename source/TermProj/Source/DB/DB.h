#pragma once
#include <iostream>
#include "../Game/Network/Network.h"
#include <sqlext.h>
#include <atlconv.h>


extern SQLHENV henv;
extern SQLHDBC hdbc; 
extern SQLHSTMT hstmt;
void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

void InitializeDB();
void ReleaseDB();
int Login(const char* name, LoginInfo& p_info);
int MakeCharacterAndLogin(const char* name, LoginInfo& p_info);
void SavePos(const char* name, int x, int y);
void SaveStatus(const char* name, short hp, short maxhp, short level, short exp,short x, short y);