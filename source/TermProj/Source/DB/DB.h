#pragma once
#include <iostream>
#include "../Game/Network/Network.h"
#include <sqlext.h>
#include <atlconv.h>


extern SQLHENV henv[6];
extern SQLHDBC hdbc[6]; 
extern SQLHSTMT hstmt[6];
void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

void InitializeDB(SQLHENV& henv, SQLHDBC& hdbc, SQLHSTMT& hstmt);
void ReleaseDB(SQLHENV& henv, SQLHDBC& hdbc, SQLHSTMT& hstmt);
int Login(const char* name, LoginInfo& p_info, SQLHSTMT& hstmt);
int MakeCharacterAndLogin(const char* name, LoginInfo& p_info, SQLHSTMT& hstmt);
void SavePos(const char* name, int x, int y, SQLHSTMT& hstmt);
void SaveStatus(const char* name, short hp, short maxhp, short level, short exp,short x, short y);