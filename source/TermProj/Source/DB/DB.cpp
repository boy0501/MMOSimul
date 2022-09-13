#include <string>
#include "DB.h"

SQLHENV henv;
SQLHDBC hdbc; 
SQLHSTMT hstmt;

using namespace std;

void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	cout << "그만들어오라" << endl;
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}
void InitializeDB()
{
	SQLRETURN retcode;
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2017182030_2021_GameServer", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					cout << "connection success!" << endl;

					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
				}
				else {
					cout << "connection fail!" << endl;
				}
			}
		}
	}
}

void ReleaseDB()
{
	SQLCancel(hstmt);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}



int Login(const char* name, LoginInfo& p_info)
{
	SQLINTEGER p_exp{};
	SQLSMALLINT p_x{}, p_y{}, p_level{}, p_hp{}, p_maxhp{};
	SQLWCHAR p_name[20]{};
	SQLLEN cbName = 0, cbP_x = 0, cbP_y = 0, cbP_exp = 0, cbP_level = 0, cbP_hp = 0, cbP_maxhp = 0;
	SQLRETURN retcode{};

	//cout << "ODBC Connected !" << endl;
	wstring LoginQuery{ L"EXEC try_login " };
	USES_CONVERSION;
	LoginQuery += A2W(name);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)LoginQuery.c_str(), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {


		// Bind columns 1, 2, and 3  
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, p_name, 20, &cbName);
		retcode = SQLBindCol(hstmt, 2, SQL_C_SHORT, &p_x, 10, &cbP_x);
		retcode = SQLBindCol(hstmt, 3, SQL_C_SHORT, &p_y, 10, &cbP_y);
		retcode = SQLBindCol(hstmt, 4, SQL_C_SHORT, &p_level, 10, &cbP_level);
		retcode = SQLBindCol(hstmt, 5, SQL_C_SHORT, &p_hp, 10, &cbP_hp);
		retcode = SQLBindCol(hstmt, 6, SQL_C_SHORT, &p_maxhp, 10, &cbP_maxhp);
		retcode = SQLBindCol(hstmt, 7, SQL_C_LONG, &p_exp, 100, &cbP_exp);

		// Fetch and print each row of data. On an error, display a message and exit.  
		//for (int i = 0; ; i++) 
		{
			retcode = SQLFetch(hstmt);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				strcpy_s(p_info.p_name, 20, (char*)p_name);
				p_info.p_x = p_x;
				p_info.p_y = p_y;
				p_info.p_level = p_level;
				p_info.p_hp = p_hp;
				p_info.p_maxhp = p_maxhp;
				p_info.p_exp = p_exp;

				SQLCancel(hstmt);
				return true;
			}
			else if (retcode == SQL_ERROR) {
				HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
				SQLCancel(hstmt);
				return -1;
			}
		}

	}
	else {
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
		// Process data  
		SQLCancel(hstmt);
		return -1;
	}

	// Process data  
	SQLCancel(hstmt);
	return -1;
}


int MakeCharacterAndLogin(const char* name, LoginInfo& p_info)
{
	SQLINTEGER p_exp{};
	SQLSMALLINT p_x{}, p_y{}, p_level{}, p_hp{}, p_maxhp{};
	SQLWCHAR p_name[20]{};
	SQLLEN cbName = 0, cbP_x = 0, cbP_y = 0, cbP_exp = 0, cbP_level = 0, cbP_hp = 0, cbP_maxhp = 0;
	SQLRETURN retcode{};

	//cout << "ODBC Connected !" << endl;
	wstring MakeCharQuery{ L"EXEC make_character " };
	wstring LoginQuery{ L"EXEC try_login " };
	USES_CONVERSION;
	LoginQuery += A2W(name);
	MakeCharQuery += A2W(name);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)MakeCharQuery.c_str(), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{

	}
	else if(retcode == SQL_ERROR){
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	}

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)LoginQuery.c_str(), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {


		// Bind columns 1, 2, and 3  
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, p_name, 20, &cbName);
		retcode = SQLBindCol(hstmt, 2, SQL_C_SHORT, &p_x, 10, &cbP_x);
		retcode = SQLBindCol(hstmt, 3, SQL_C_SHORT, &p_y, 10, &cbP_y);
		retcode = SQLBindCol(hstmt, 4, SQL_C_SHORT, &p_level, 10, &cbP_level);
		retcode = SQLBindCol(hstmt, 5, SQL_C_SHORT, &p_hp, 10, &cbP_hp);
		retcode = SQLBindCol(hstmt, 6, SQL_C_SHORT, &p_maxhp, 10, &cbP_maxhp);
		retcode = SQLBindCol(hstmt, 7, SQL_C_LONG, &p_exp, 100, &cbP_exp);

		// Fetch and print each row of data. On an error, display a message and exit.  
		//for (int i = 0; ; i++)
		{
			retcode = SQLFetch(hstmt);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				strcpy_s(p_info.p_name, 20, (char*)p_name);
				p_info.p_x = p_x;
				p_info.p_y = p_y;
				p_info.p_level = p_level;
				p_info.p_hp = p_hp;
				p_info.p_maxhp = p_maxhp;
				p_info.p_exp = p_exp;

				SQLCancel(hstmt);
				return true;
			}
			else if (retcode == SQL_ERROR) {

				HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
				SQLCancel(hstmt);
				return -1;
			}
		}

	}
	else {
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
		// Process data  
		SQLCancel(hstmt);
		return -1;
	}

	// Process data  
	SQLCancel(hstmt);
	return -1;
}



void SavePos(const char* name, int x, int y)
{
	SQLRETURN retcode;

	//cout << "ODBC Connected !" << endl;
	wstring SaveQuery{ L"EXEC save_pos " };

	USES_CONVERSION;
	SaveQuery += A2W(name);
	SaveQuery += L",";
	SaveQuery += to_wstring(x);
	SaveQuery += L",";
	SaveQuery += to_wstring(y);


	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)SaveQuery.c_str(), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		//SQLCancel(hstmt);
		//SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}
	else {
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	}

}


void SaveStatus(const char* name, short hp, short maxhp, short level, short exp,short x,short y)
{
	SQLRETURN retcode;

	//cout << "ODBC Connected !" << endl;
	wstring SaveQuery{ L"EXEC save_status " };

	USES_CONVERSION;
	SaveQuery += A2W(name);
	SaveQuery += L",";
	SaveQuery += to_wstring(hp);
	SaveQuery += L",";
	SaveQuery += to_wstring(maxhp);
	SaveQuery += L",";
	SaveQuery += to_wstring(level);
	SaveQuery += L",";
	SaveQuery += to_wstring(exp);
	SaveQuery += L",";
	SaveQuery += to_wstring(x);
	SaveQuery += L",";
	SaveQuery += to_wstring(y);


	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)SaveQuery.c_str(), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
	}
	else {
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	}
}