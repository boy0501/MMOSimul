#include <string>
#include "DB.h"

//#pragma comment (lib, "libmysql.lib")

MYSQL* hmysql;

using namespace std;

void PrepareBindParam(const enum_field_types& type,MYSQL_BIND& bind_param,void* buffer,unsigned long buffer_len,unsigned long& res_len,bool* is_null,bool* error)
{
	bind_param.buffer_type = type;
	bind_param.buffer = (char*)buffer;
	bind_param.buffer_length = buffer_len;
	bind_param.length = &res_len;
	bind_param.is_null = is_null;
	bind_param.error = error;
}

void InitializeDB()
{
	MYSQL* hmysql;
	MYSQL* conn_result;
	unsigned int timeout_sec = 1;

	hmysql = mysql_init(NULL);
	mysql_options(hmysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout_sec);
	conn_result = mysql_real_connect(hmysql, "localhost", "root", "dltnals", "simplemmo", 3306, NULL, 0);

	if (NULL == conn_result)
	{
		cout << "DB Connection Fail" << endl;
	}
	else
	{
		cout << "DB Connection Success" << endl;

		char query[1024];
		MYSQL_RES* result;
		MYSQL_ROW row;

		string id{ "test1" }, pw{ "1234" };
		sprintf_s(query, 1024, "call try_login('%s','%s')", id.c_str(), pw.c_str());
		//int res = sprintf_s(query, 1024, "SELECT * from player_data WHERE player_name = '%s' and player_password = '%s'", id.c_str(), pw.c_str());
		// Send Query
		//if (mysql_query(hmysql, query))
		//{
		//	cout << "SELECT Fail" << endl;
		//	fprintf(stderr, "Error %d\n%s", mysql_errno(hmysql), mysql_error(hmysql));
		//	return;
		//}
		
		
		
		//-----------------------------------------
		char stmtquery[1024];
		sprintf_s(stmtquery, 1024, "call try_login(?,?)");
		auto pstmt = mysql_stmt_init(hmysql);	//prepare statement 생성
		if (mysql_stmt_prepare(pstmt, stmtquery, strlen(stmtquery)))	// prepare statment 쿼리
		{
			fprintf(stderr, " mysql_stmt_prepare(), INSERT failed\n");
			fprintf(stderr, " %s\n", mysql_stmt_error(pstmt));
			fprintf(stderr, " %ld\n", mysql_stmt_errno(pstmt));
		}

		MYSQL_BIND bind_parameter[2];
		string b_name,b_pw;
		unsigned long b_namelen{}, b_pwlen{};
		PrepareBindParam(MYSQL_TYPE_STRING, bind_parameter[0], const_cast<char*>(b_name.c_str()), 20, b_namelen, 0, 0);
		PrepareBindParam(MYSQL_TYPE_STRING, bind_parameter[1], const_cast<char*>(b_pw.c_str()), 20, b_pwlen, 0, 0);
		
		// 버퍼 바인딩
		if (mysql_stmt_bind_param(pstmt, bind_parameter))
		{
			fprintf(stderr, " mysql_stmt_bind_param() failed\n");
			fprintf(stderr, " %s\n", mysql_stmt_error(pstmt));
		}

		b_name = "test1";
		b_namelen = b_name.length();
		b_pw = "1234";
		b_pwlen = b_pw.length();
		if (mysql_stmt_execute(pstmt))
		{
			fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
			fprintf(stderr, " %s\n", mysql_stmt_error(pstmt));
		}
		
		/* Fetch result set meta information */
		auto prepare_meta_result = mysql_stmt_result_metadata(pstmt);
		if (!prepare_meta_result)
		{
			fprintf(stderr,
				" mysql_stmt_result_metadata(), \
           returned no meta information\n");
			fprintf(stderr, " %s\n", mysql_stmt_error(pstmt));
		}


		/* Bind the result buffers for all 4 columns before fetching them */
		MYSQL_BIND bind_res[8];
		bool bind_error[8];
		bool bind_isnull[8];
		unsigned long bind_reslen[8];

		memset(bind_res, 0, sizeof(bind_res));

		string r_name,r_pw;
		short r_x{}, r_y{}, r_lv{}, r_hp{}, r_mhp{};
		int r_exp{};
		PrepareBindParam(MYSQL_TYPE_STRING, bind_res[0], const_cast<char*>(r_name.c_str()), 20, bind_reslen[0], &bind_isnull[0], &bind_error[0]);
		PrepareBindParam(MYSQL_TYPE_STRING, bind_res[1], const_cast<char*>(r_pw.c_str()), 20, bind_reslen[1], &bind_isnull[1], &bind_error[1]);
		PrepareBindParam(MYSQL_TYPE_SHORT, bind_res[2], &r_y, 0, bind_reslen[2], &bind_isnull[2], &bind_error[2]);
		PrepareBindParam(MYSQL_TYPE_SHORT, bind_res[3], &r_y, 0, bind_reslen[3], &bind_isnull[3], &bind_error[3]);
		PrepareBindParam(MYSQL_TYPE_SHORT, bind_res[4], &r_lv, 0, bind_reslen[4], &bind_isnull[4], &bind_error[4]);
		PrepareBindParam(MYSQL_TYPE_SHORT, bind_res[5], &r_hp, 0, bind_reslen[5], &bind_isnull[5], &bind_error[5]);
		PrepareBindParam(MYSQL_TYPE_SHORT, bind_res[6], &r_mhp, 0, bind_reslen[6], &bind_isnull[6], &bind_error[6]);
		PrepareBindParam(MYSQL_TYPE_LONG, bind_res[7], &r_exp, 0, bind_reslen[7], &bind_isnull[7], &bind_error[7]);

		/* Bind the result buffers */
		if (mysql_stmt_bind_result(pstmt, bind_res))
		{
			fprintf(stderr, " mysql_stmt_bind_result() failed\n");
			fprintf(stderr, " %s\n", mysql_stmt_error(pstmt));
			fprintf(stderr, " %ld\n", mysql_stmt_errno(pstmt));
		}

		/* Now buffer all results to client (optional step) */
		// fetch할때 서버에서 하나씩 받아오는게 아닌, 한방에 버퍼에 넣어두기 위한 작업
		// 결과 집합을 생성하지 않는 SQL 문을 실행한 후 호출할 필요는 없지만 호출하더라도 성능에 해를 끼치거나 눈에 띄는 성능 문제가 발생하지는 않습니다. -mysql 공식문서
		if (mysql_stmt_store_result(pstmt))
		{
			fprintf(stderr, " mysql_stmt_store_result() failed\n");
			fprintf(stderr, " %s\n", mysql_stmt_error(pstmt));
			
		}

		int row_count = 0;
		while (!mysql_stmt_fetch(pstmt))
		{
			row_count++;

		}

		/* Close the statement */
		if (mysql_stmt_close(pstmt))
		{
			/* mysql_stmt_close() invalidates stmt, so call          */
			/* mysql_error(mysql) rather than mysql_stmt_error(stmt) */
			fprintf(stderr, " failed while closing the statement\n");
			fprintf(stderr, " %s\n", mysql_error(hmysql));
		}


		//--------------------------------------

		// Get Response
		result = mysql_store_result(hmysql);

		MYSQL_FIELD* field;
		field = mysql_fetch_fields(result);

		int fields = mysql_num_fields(result);    // 필드 갯수 구함
		LoginInfo a;
		while (row = mysql_fetch_row(result))     // 모든 레코드 탐색 실패시 NULL반환 while 탈출
		{
			for (int i = 0; i < fields; i++)    // 각각의 레코드에서 모든 필드 값 출력
			{
				cout << field[i].name<<":" << row[i] << "   \n";
			}
			cout << endl;
		}

		mysql_free_result(result);
		mysql_close(hmysql);
	}
}

void ReleaseDB()
{
	mysql_close(hmysql);
}



int Login(const char* name, const char* pw, LoginInfo& p_info)
{

	char query[1024];
	MYSQL_RES* result;
	MYSQL_ROW row;

	sprintf_s(query, 1024, "call try_login('%s','%s')", name, pw);
	//int res = sprintf_s(query, 1024, "SELECT * from player_data WHERE player_name = '%s' and player_password = '%s'", id.c_str(), pw.c_str());
	// Send Query
	if (mysql_query(hmysql, query))
	{
		cout << "SELECT Fail" << endl;
		fprintf(stderr, "Error %d\n%s", mysql_errno(hmysql), mysql_error(hmysql));
		return -1;
	}

	// Get Response
	result = mysql_store_result(hmysql);

	int fields = mysql_num_fields(result);    // 필드 갯수 구함
	
	while (row = mysql_fetch_row(result))     // 모든 레코드 탐색
	{
		for (int i = 0; i < fields; i++)    // 각각의 레코드에서 모든 필드 값 출력
		{
			cout << row[i] << "   ";
		}
		cout << endl;
	}

	mysql_free_result(result);


	//SQLINTEGER p_exp{};
	//SQLSMALLINT p_x{}, p_y{}, p_level{}, p_hp{}, p_maxhp{};
	//SQLWCHAR p_name[20]{};
	//SQLLEN cbName = 0, cbP_x = 0, cbP_y = 0, cbP_exp = 0, cbP_level = 0, cbP_hp = 0, cbP_maxhp = 0;
	//SQLRETURN retcode{};

	////cout << "ODBC Connected !" << endl;
	//wstring LoginQuery{ L"EXEC try_login " };
	//USES_CONVERSION;
	//LoginQuery += A2W(name);

	//retcode = SQLExecDirect(hstmt, (SQLWCHAR*)LoginQuery.c_str(), SQL_NTS);
	//if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {


	//	// Bind columns 1, 2, and 3  
	//	retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, p_name, 20, &cbName);
	//	retcode = SQLBindCol(hstmt, 2, SQL_C_SHORT, &p_x, 10, &cbP_x);
	//	retcode = SQLBindCol(hstmt, 3, SQL_C_SHORT, &p_y, 10, &cbP_y);
	//	retcode = SQLBindCol(hstmt, 4, SQL_C_SHORT, &p_level, 10, &cbP_level);
	//	retcode = SQLBindCol(hstmt, 5, SQL_C_SHORT, &p_hp, 10, &cbP_hp);
	//	retcode = SQLBindCol(hstmt, 6, SQL_C_SHORT, &p_maxhp, 10, &cbP_maxhp);
	//	retcode = SQLBindCol(hstmt, 7, SQL_C_LONG, &p_exp, 100, &cbP_exp);

	//	// Fetch and print each row of data. On an error, display a message and exit.  
	//	//for (int i = 0; ; i++) 
	//	{
	//		retcode = SQLFetch(hstmt);
	//		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	//		{
	//			strcpy_s(p_info.p_name, 20, (char*)p_name);
	//			p_info.p_x = p_x;
	//			p_info.p_y = p_y;
	//			p_info.p_level = p_level;
	//			p_info.p_hp = p_hp;
	//			p_info.p_maxhp = p_maxhp;
	//			p_info.p_exp = p_exp;

	//			SQLCancel(hstmt);
	//			return true;
	//		}
	//		else if (retcode == SQL_ERROR) {
	//			HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	//			SQLCancel(hstmt);
	//			return -1;
	//		}
	//	}

	//}
	//else {
	//	HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	//	// Process data  
	//	SQLCancel(hstmt);
	//	return -1;
	//}

	//// Process data  
	//SQLCancel(hstmt);
	//return -1;
}


int MakeCharacterAndLogin(const char* name, LoginInfo& p_info)
{
//	SQLINTEGER p_exp{};
//	SQLSMALLINT p_x{}, p_y{}, p_level{}, p_hp{}, p_maxhp{};
//	SQLWCHAR p_name[20]{};
//	SQLLEN cbName = 0, cbP_x = 0, cbP_y = 0, cbP_exp = 0, cbP_level = 0, cbP_hp = 0, cbP_maxhp = 0;
//	SQLRETURN retcode{};
//
//	//cout << "ODBC Connected !" << endl;
//	wstring MakeCharQuery{ L"EXEC make_character " };
//	wstring LoginQuery{ L"EXEC try_login " };
//	USES_CONVERSION;
//	LoginQuery += A2W(name);
//	MakeCharQuery += A2W(name);
//
//	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)MakeCharQuery.c_str(), SQL_NTS);
//	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
//	{
//
//	}
//	else if(retcode == SQL_ERROR){
//		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
//	}
//
//	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)LoginQuery.c_str(), SQL_NTS);
//	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
//
//
//		// Bind columns 1, 2, and 3  
//		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, p_name, 20, &cbName);
//		retcode = SQLBindCol(hstmt, 2, SQL_C_SHORT, &p_x, 10, &cbP_x);
//		retcode = SQLBindCol(hstmt, 3, SQL_C_SHORT, &p_y, 10, &cbP_y);
//		retcode = SQLBindCol(hstmt, 4, SQL_C_SHORT, &p_level, 10, &cbP_level);
//		retcode = SQLBindCol(hstmt, 5, SQL_C_SHORT, &p_hp, 10, &cbP_hp);
//		retcode = SQLBindCol(hstmt, 6, SQL_C_SHORT, &p_maxhp, 10, &cbP_maxhp);
//		retcode = SQLBindCol(hstmt, 7, SQL_C_LONG, &p_exp, 100, &cbP_exp);
//
//		// Fetch and print each row of data. On an error, display a message and exit.  
//		//for (int i = 0; ; i++)
//		{
//			retcode = SQLFetch(hstmt);
//			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
//			{
//				strcpy_s(p_info.p_name, 20, (char*)p_name);
//				p_info.p_x = p_x;
//				p_info.p_y = p_y;
//				p_info.p_level = p_level;
//				p_info.p_hp = p_hp;
//				p_info.p_maxhp = p_maxhp;
//				p_info.p_exp = p_exp;
//
//				SQLCancel(hstmt);
//				return true;
//			}
//			else if (retcode == SQL_ERROR) {
//
//				HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
//				SQLCancel(hstmt);
//				return -1;
//			}
//		}
//
//	}
//	else {
//		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
//		// Process data  
//		SQLCancel(hstmt);
//		return -1;
//	}
//
//	// Process data  
//	SQLCancel(hstmt);
//	return -1;
	return -1;
}



void SavePos(const char* name, int x, int y)
{
//	SQLRETURN retcode;
//
//	//cout << "ODBC Connected !" << endl;
//	wstring SaveQuery{ L"EXEC save_pos " };
//
//	USES_CONVERSION;
//	SaveQuery += A2W(name);
//	SaveQuery += L",";
//	SaveQuery += to_wstring(x);
//	SaveQuery += L",";
//	SaveQuery += to_wstring(y);
//
//
//	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)SaveQuery.c_str(), SQL_NTS);
//	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
//		//SQLCancel(hstmt);
//		//SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
//	}
//	else {
//		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
//	}
//
}
//
//
void SaveStatus(const char* name, short hp, short maxhp, short level, short exp,short x,short y)
{
//	SQLRETURN retcode;
//
//	//cout << "ODBC Connected !" << endl;
//	wstring SaveQuery{ L"EXEC save_status " };
//
//	USES_CONVERSION;
//	SaveQuery += A2W(name);
//	SaveQuery += L",";
//	SaveQuery += to_wstring(hp);
//	SaveQuery += L",";
//	SaveQuery += to_wstring(maxhp);
//	SaveQuery += L",";
//	SaveQuery += to_wstring(level);
//	SaveQuery += L",";
//	SaveQuery += to_wstring(exp);
//	SaveQuery += L",";
//	SaveQuery += to_wstring(x);
//	SaveQuery += L",";
//	SaveQuery += to_wstring(y);
//
//
//	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)SaveQuery.c_str(), SQL_NTS);
//	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
//	}
//	else {
//		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
//	}
}