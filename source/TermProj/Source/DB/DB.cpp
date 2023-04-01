#include <string>
#include "DB.h"

//#pragma comment (lib, "libmysql.lib")

MYSQL* hmysql;

using namespace std;

void PrepareBindParam(const enum_field_types& type,MYSQL_BIND& bind_param,void* buffer,unsigned long buffer_len,unsigned long* res_len,bool* is_null,bool* error)
{
	bind_param.buffer_type = type;
	bind_param.buffer = (char*)buffer;
	bind_param.buffer_length = buffer_len;
	bind_param.length = res_len;
	bind_param.is_null = is_null;
	bind_param.error = error;
}

void StmtErrorCheck(MYSQL_STMT* pstmt, const char* funcname)
{
	fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
	fprintf(stderr, " %s\n", mysql_stmt_error(pstmt));
	fprintf(stderr, " %ld\n", mysql_stmt_errno(pstmt));
}

void pstmtObjectExample()
{
	MYSQL_BIND bind_parameter[3];
	string b_name, b_pw;
	short is_ok;
	unsigned long b_namelen{}, b_pwlen{}, b_isoklen;
	PrepareBindParam(MYSQL_TYPE_STRING, bind_parameter[0], const_cast<char*>(b_name.c_str()), 20, &b_namelen, 0, 0);
	PrepareBindParam(MYSQL_TYPE_STRING, bind_parameter[1], const_cast<char*>(b_pw.c_str()), 20, &b_pwlen, 0, 0);
	PrepareBindParam(MYSQL_TYPE_SHORT, bind_parameter[2], &is_ok, 0, &b_isoklen, 0, 0);
	b_name = "test2";
	b_namelen = b_name.length();
	b_pw = "1234";
	b_pwlen = b_pw.length();

	MYSQL_BIND bind_res[8];
	bool bind_error[8];
	bool bind_isnull[8];
	unsigned long bind_reslen[8];

	memset(bind_res, 0, sizeof(bind_res));

	string r_name, r_pw;
	short r_x{}, r_y{}, r_lv{}, r_hp{}, r_mhp{};
	int r_exp{};
	PrepareBindParam(MYSQL_TYPE_STRING, bind_res[0], const_cast<char*>(r_name.c_str()), 20, &bind_reslen[0], &bind_isnull[0], &bind_error[0]);
	PrepareBindParam(MYSQL_TYPE_STRING, bind_res[1], const_cast<char*>(r_pw.c_str()), 20, &bind_reslen[1], &bind_isnull[1], &bind_error[1]);
	PrepareBindParam(MYSQL_TYPE_SHORT, bind_res[2], &r_x, 0, &bind_reslen[2], &bind_isnull[2], &bind_error[2]);
	PrepareBindParam(MYSQL_TYPE_SHORT, bind_res[3], &r_y, 0, &bind_reslen[3], &bind_isnull[3], &bind_error[3]);
	PrepareBindParam(MYSQL_TYPE_SHORT, bind_res[4], &r_lv, 0, &bind_reslen[4], &bind_isnull[4], &bind_error[4]);
	PrepareBindParam(MYSQL_TYPE_SHORT, bind_res[5], &r_hp, 0, &bind_reslen[5], &bind_isnull[5], &bind_error[5]);
	PrepareBindParam(MYSQL_TYPE_SHORT, bind_res[6], &r_mhp, 0, &bind_reslen[6], &bind_isnull[6], &bind_error[6]);
	PrepareBindParam(MYSQL_TYPE_LONG, bind_res[7], &r_exp, 0, &bind_reslen[7], &bind_isnull[7], &bind_error[7]);


	{
		PrepareStatement pstmt{ hmysql };
		pstmt.SetQuery("Call dummytest(?,?,?)");
		pstmt.AddDataParameter(bind_parameter);
		pstmt.Excute();
		pstmt.AddResultParameter(bind_res);
		do {
			pstmt.ReadyToResponse();
			while (!pstmt.Fetch())
			{
				cout << r_name.c_str() << " " << r_pw.c_str() << " " << r_x << "," << r_y << "," << r_lv << "," << r_hp << "," << r_mhp << "," << r_exp << endl;
				//Do Something;
			}
		} while (pstmt.GetResponse() == 0);
	}

	{
		PrepareStatement pstmt{ hmysql };
		pstmt.SetQuery("Call dummytest(?,?,?)");
		pstmt.AddDataParameter(bind_parameter);
		pstmt.Excute();
		pstmt.AddResultParameter(bind_res);
		do {
			pstmt.ReadyToResponse();
			while (!pstmt.Fetch())
			{
				//Do Something;
			}
		} while (pstmt.GetResponse() == 0);
	}

}

void pstmtExample(MYSQL* hmysql)
{
	pstmtObjectExample();

	//-----------------------------------------
	char stmtquery[1024];
	int status = 0;
	sprintf_s(stmtquery, 1024, "call dummytest(?,?,?)");
	//sprintf_s(stmtquery, 1024, "call make_character(?,?)");
	//sprintf_s(stmtquery, 1024, "SELECT * from player_data");
	//sprintf_s(stmtquery, 1024, "SELECT * from player_data WHERE player_name = ? and player_password = ?");
	auto pstmt = mysql_stmt_init(hmysql);	//prepare statement 생성



	if (mysql_stmt_prepare(pstmt, stmtquery, strlen(stmtquery)))	// prepare statment 쿼리
	{
		fprintf(stderr, " mysql_stmt_prepare(), failed\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(pstmt));
		fprintf(stderr, " %ld\n", mysql_stmt_errno(pstmt));
	}

	MYSQL_BIND bind_parameter[3];
	string b_name,b_pw;
	short is_ok;
	unsigned long b_namelen{}, b_pwlen{},b_isoklen;
	PrepareBindParam(MYSQL_TYPE_STRING, bind_parameter[0], const_cast<char*>(b_name.c_str()), 20, &b_namelen, 0, 0);
	PrepareBindParam(MYSQL_TYPE_STRING, bind_parameter[1], const_cast<char*>(b_pw.c_str()), 20, &b_pwlen, 0, 0);
	PrepareBindParam(MYSQL_TYPE_SHORT, bind_parameter[2], &is_ok, 0, &b_isoklen, 0, 0);
	
	// 버퍼 바인딩
	if (mysql_stmt_bind_param(pstmt, bind_parameter))
	{
		fprintf(stderr, " mysql_stmt_bind_param() failed\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(pstmt));
	}
	
	b_name = "test2";
	b_namelen = b_name.length();
	b_pw = "1234";
	b_pwlen = b_pw.length();

	if (mysql_stmt_execute(pstmt))
	{
		fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(pstmt));
	}
	do {

		int	num_fields = mysql_stmt_field_count(pstmt);
		if (num_fields > 0)
		{
			/* Bind the result buffers for all 4 columns before fetching them */
			MYSQL_BIND bind_res[8];
			bool bind_error[8];
			bool bind_isnull[8];
			unsigned long bind_reslen[8];

			memset(bind_res, 0, sizeof(bind_res));

			string r_name, r_pw;
			short r_x{}, r_y{}, r_lv{}, r_hp{}, r_mhp{};
			int r_exp{};
			PrepareBindParam(MYSQL_TYPE_STRING, bind_res[0], const_cast<char*>(r_name.c_str()), 20, &bind_reslen[0], &bind_isnull[0], &bind_error[0]);
			PrepareBindParam(MYSQL_TYPE_STRING, bind_res[1], const_cast<char*>(r_pw.c_str()), 20, &bind_reslen[1], &bind_isnull[1], &bind_error[1]);
			PrepareBindParam(MYSQL_TYPE_SHORT, bind_res[2], &r_x, 0, &bind_reslen[2], &bind_isnull[2], &bind_error[2]);
			PrepareBindParam(MYSQL_TYPE_SHORT, bind_res[3], &r_y, 0, &bind_reslen[3], &bind_isnull[3], &bind_error[3]);
			PrepareBindParam(MYSQL_TYPE_SHORT, bind_res[4], &r_lv, 0, &bind_reslen[4], &bind_isnull[4], &bind_error[4]);
			PrepareBindParam(MYSQL_TYPE_SHORT, bind_res[5], &r_hp, 0, &bind_reslen[5], &bind_isnull[5], &bind_error[5]);
			PrepareBindParam(MYSQL_TYPE_SHORT, bind_res[6], &r_mhp, 0, &bind_reslen[6], &bind_isnull[6], &bind_error[6]);
			PrepareBindParam(MYSQL_TYPE_LONG, bind_res[7], &r_exp, 0, &bind_reslen[7], &bind_isnull[7], &bind_error[7]);


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
			/* Free the prepared result metadata */
			auto infos = mysql_stmt_affected_rows(pstmt);
			cout << "변경된거 :" << row_count << endl;

			
		}

		status = mysql_stmt_next_result(pstmt);
		if (status > 0)
		{
			fprintf(stderr, " %s\n", mysql_stmt_error(pstmt));			
		}
	} while (status == 0);
	
	/* Close the statement */
	if (mysql_stmt_close(pstmt))
	{
		/* mysql_stmt_close() invalidates stmt, so call          */
		/* mysql_error(mysql) rather than mysql_stmt_error(stmt) */
		fprintf(stderr, " failed while closing the statement\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(pstmt));
		fprintf(stderr, " %s\n", mysql_error(hmysql));
		fprintf(stderr, " %ld\n", mysql_errno(hmysql));
	}


	//--------------------------------------
}

void HowtoOUTparamForStoredProcedure(MYSQL* hmysql)
{

	char query[1024];
	MYSQL_RES* result{};
	MYSQL_ROW row;
	//pstmtExample(hmysql);
	string id{ "test1" }, pw{ "1234" };
	short is_ok{};
	sprintf_s(query, 1024, "call dummytest('%s','%s',@param3)", id.c_str(), pw.c_str());
	//sprintf_s(query, 1024, "call try_login('%s','%s')", id.c_str(), pw.c_str());
	//sprintf_s(query, 1024, "call dummytest()");
	//int res = sprintf_s(query, 1024, "SELECT * from player_data WHERE player_name = '%s' and player_password = '%s'", id.c_str(), pw.c_str());
	//Send Query
	if (mysql_query(hmysql, query))
	{
		cout << "SELECT Fail" << endl;
		fprintf(stderr, "Error %d\n%s", mysql_errno(hmysql), mysql_error(hmysql));
		return;
	}
	// 
	// Get Response
	//result = mysql_store_result(hmysql);
	//
	//MYSQL_FIELD* field;
	//field = mysql_fetch_fields(result);
	//
	//int fields = mysql_num_fields(result);    // 필드 갯수 구함
	//LoginInfo a;
	//while (row = mysql_fetch_row(result))     // 모든 레코드 탐색 실패시 NULL반환 while 탈출
	//{
	//	for (int i = 0; i < fields; i++)    // 각각의 레코드에서 모든 필드 값 출력
	//	{
	//		cout << field[i].name<<":" << row[i] << "   \n";
	//	}
	//	cout << endl;
	//}

	mysql_free_result(result);


	sprintf_s(query, 1024, "select @param3", id.c_str());
	if (mysql_query(hmysql, query))
	{
		cout << "SELECT Fail" << endl;
		fprintf(stderr, "Error %d\n%s", mysql_errno(hmysql), mysql_error(hmysql));
		return;
	}

	result = mysql_store_result(hmysql);

	MYSQL_FIELD* field;
	field = mysql_fetch_fields(result);

	int fields = mysql_num_fields(result);    // 필드 갯수 구함
	LoginInfo a;
	while (row = mysql_fetch_row(result))     // 모든 레코드 탐색 실패시 NULL반환 while 탈출
	{
		for (int i = 0; i < fields; i++)    // 각각의 레코드에서 모든 필드 값 출력
		{
			cout << field[i].name << ":" << row[i] << "   \n";
		}
		cout << endl;
	}

	mysql_free_result(result);
}

void mysqlTestCode()
{
	//MYSQL* hmysql;
	MYSQL* conn_result;
	unsigned int timeout_sec = 1;

	hmysql = mysql_init(NULL);
	mysql_options(hmysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout_sec);
	conn_result = mysql_real_connect(hmysql, "localhost", "root", "dltnals", "simplemmo", 3306, NULL, CLIENT_MULTI_STATEMENTS);

	if (NULL == conn_result)
	{
		cout << "DB Connection Fail" << endl;
	}
	else
	{
		cout << "DB Connection Success" << endl;
		LoginInfo tmp;
		char query[1024];
		MYSQL_RES* result{};
		MYSQL_ROW row;
		//pstmtExample(hmysql);
		string id{ "sponge" }, pw{ "12354" };
		short is_ok{};
		//sprintf_s(query, 1024, "call dummytest('%s','%s')", id.c_str(), pw.c_str());
		//sprintf_s(query, 1024, "call save_status('%s',%d,%d,%d,%d,%d,%d)", id.c_str(), 20, 20, 1, 0, 10, 10);
		sprintf_s(query, 1024, "call try_login('%s','%s')", id.c_str(), pw.c_str());
		//sprintf_s(query, 1024, "call dummytest()");
		//int res = sprintf_s(query, 1024, "SELECT * from player_data WHERE player_name = '%s' and player_password = '%s'", id.c_str(), pw.c_str());
		//Send Query
		int status = 0;
		if (status = mysql_query(hmysql, query))
		{
			cout << "SELECT Fail" << endl;
			fprintf(stderr, "Error %d\n%s", mysql_errno(hmysql), mysql_error(hmysql));
			return;
		}
		// 
		// Get Response
		result = mysql_store_result(hmysql);
		cout << mysql_affected_rows(hmysql) << endl;

		MYSQL_FIELD* field;
		field = mysql_fetch_fields(result);
		int fields = mysql_num_fields(result);    // 필드 갯수 구함
		LoginInfo a;
		while (row = mysql_fetch_row(result))     // 모든 레코드 탐색 실패시 NULL반환 while 탈출
		{
			for (int i = 0; i < fields; i++)    // 각각의 레코드에서 모든 필드 값 출력
			{
				cout << field[i].name << ":" << row[i] << "   \n";
			}
			cout << endl;
		}

		if ((status = mysql_next_result(hmysql)) > 0)
			printf("Could not execute statement\n");
		mysql_free_result(result);
		if (mysql_query(hmysql, query))
		{
			cout << "SELECT Fail" << endl;
			fprintf(stderr, "Error %d\n%s", mysql_errno(hmysql), mysql_error(hmysql));
			return;
		}

		mysql_close(hmysql);
	}
}


void InitializeDB()
{
	MYSQL* conn_result;
	unsigned int timeout_sec = 1;

	hmysql = mysql_init(NULL);
	mysql_options(hmysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout_sec);
	conn_result = mysql_real_connect(hmysql, "localhost", "root", "dltnals", "simplemmo", 3306, NULL, CLIENT_MULTI_STATEMENTS);

	if (NULL == conn_result)
	{
		cout << "DB Connection Fail" << endl;
	}
	else
	{
		cout << "DB Connection Success" << endl;
		pstmtExample(hmysql);
	}
}

void ReleaseDB()
{
	mysql_close(hmysql);
}



int Login(const char* name, const char* pw, LoginInfo& p_info)
{

	char query[1024];
	MYSQL_RES* result{};
	MYSQL_ROW row;
	short is_ok{};
	sprintf_s(query, 1024, "call try_login('%s','%s')", name, pw);
	if (mysql_query(hmysql, query))
	{
		cout << "SELECT Fail" << endl;
		fprintf(stderr, "Error %d\n%s", mysql_errno(hmysql), mysql_error(hmysql));
		return -1;
	}
	// 
	// Get Response
	result = mysql_store_result(hmysql);
	cout << mysql_affected_rows(hmysql) << endl;
	if (mysql_affected_rows(hmysql) == 1)
	{
		MYSQL_FIELD* field;
		field = mysql_fetch_fields(result);
		int fields = mysql_num_fields(result);    // 필드 갯수 구함
		LoginInfo a;
		row = mysql_fetch_row(result);     // 모든 레코드 탐색 실패시 NULL반환 while 탈출

		strcpy_s(p_info.p_name, 20, row[0]);
		p_info.p_x = atoi(row[2]);
		p_info.p_y = atoi(row[3]);
		p_info.p_level = atoi(row[4]);
		p_info.p_hp = atoi(row[5]);
		p_info.p_maxhp = atoi(row[6]);
		p_info.p_exp = atoi(row[7]);


		mysql_free_result(result);


		if (mysql_next_result(hmysql) >= 0)
			fprintf(stderr, "more result or error\n");

		return 1;				//로그인 성공
	}
	else {

		mysql_free_result(result);
		return -1;				//로그인 실패 
	}

}


int MakeCharacterAndLogin(const char* name,const char* pw, LoginInfo& p_info)
{


	char makecharquery[1024];
	char loginquery[1024];
	MYSQL_RES* result{};
	MYSQL_ROW row;
	short is_ok{};
	if (pw == nullptr)
	{
		sprintf_s(makecharquery, 1024, "call make_character('%s','%s')", name, "1234");
		sprintf_s(loginquery, 1024, "call try_login('%s','%s')", name, "1234");
	}
	else {
		sprintf_s(makecharquery, 1024, "call make_character('%s','%s')", name, pw);
		sprintf_s(loginquery, 1024, "call try_login('%s','%s')", name, pw);
	}

	if (mysql_query(hmysql, makecharquery))
	{
		cout << "make character Fail" << endl;
		fprintf(stderr, "Error %d\n%s", mysql_errno(hmysql), mysql_error(hmysql));
		return -1;
	}

	if (mysql_next_result(hmysql) >= 0)
		fprintf(stderr, "more result or error\n");

	if (mysql_query(hmysql, loginquery))
	{
		cout << "try login Fail" << endl;
		fprintf(stderr, "Error %d\n%s", mysql_errno(hmysql), mysql_error(hmysql));
		return -1;
	}
	// Get Response
	result = mysql_store_result(hmysql);
	cout << mysql_affected_rows(hmysql) << endl;
	if (mysql_affected_rows(hmysql) == 1)
	{
		MYSQL_FIELD* field;
		field = mysql_fetch_fields(result);
		int fields = mysql_num_fields(result);    // 필드 갯수 구함
		LoginInfo a;
		row = mysql_fetch_row(result);     // 모든 레코드 탐색 실패시 NULL반환 while 탈출

		strcpy_s(p_info.p_name, 20, row[0]);
		p_info.p_x = atoi(row[2]);
		p_info.p_y = atoi(row[3]);
		p_info.p_level = atoi(row[4]);
		p_info.p_hp = atoi(row[5]);
		p_info.p_maxhp = atoi(row[6]);
		p_info.p_exp = atoi(row[7]);


		mysql_free_result(result);


		if (mysql_next_result(hmysql) >= 0)
			fprintf(stderr, "more result or error\n");


		return 1;				//로그인 성공
	}
	else {

		mysql_free_result(result);

		if (mysql_next_result(hmysql) >= 0)
			fprintf(stderr, "more result or error\n");
		return -1;				//로그인 실패 
	}
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

	char query[1024];
	sprintf_s(query, 1024, "call save_status('%s',%d,%d,%d,%d,%d,%d)", name, hp, maxhp, level, exp, x, y);
	if (mysql_query(hmysql, query))
	{
		cout << "save status Fail" << endl;
		fprintf(stderr, "Error %d\n%s", mysql_errno(hmysql), mysql_error(hmysql));
		return;
	}

	if (mysql_next_result(hmysql) >= 0)
		fprintf(stderr, "more result or error\n");
	return ;
}

PrepareStatement::PrepareStatement(MYSQL* sql)
	:mAffectRowCnt(0)
	, mQueryType(0)
{
	mMysql = sql;
	mPstmt = mysql_stmt_init(mMysql);
}

PrepareStatement::~PrepareStatement()
{
	if (mysql_stmt_close(mPstmt))
	{
		/* mysql_stmt_close() invalidates stmt, so call          */
		/* mysql_error(mysql) rather than mysql_stmt_error(stmt) */
		fprintf(stderr, " failed while closing the statement\n");
		fprintf(stderr, " %s\n", mysql_error(mMysql));
		fprintf(stderr, " %ld\n", mysql_errno(mMysql));
	}
}

void PrepareStatement::SetQuery(const char* query)
{
	mQuery = query;
	SetQueryType();

	if (mysql_stmt_prepare(mPstmt, mQuery.c_str(), mQuery.length()))	// prepare statment 쿼리
	{
		fprintf(stderr, " mysql_stmt_prepare(), failed\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(mPstmt));
		fprintf(stderr, " %ld\n", mysql_stmt_errno(mPstmt));
	}
}

void PrepareStatement::AddDataParameter(MYSQL_BIND* params)
{
	if (mysql_stmt_bind_param(mPstmt, params))
	{
		fprintf(stderr, " mysql_stmt_bind_param() failed\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(mPstmt));
	}
}

void PrepareStatement::AddResultParameter(MYSQL_BIND* params)
{
	/* Bind the result buffers */
	if (mysql_stmt_bind_result(mPstmt, params))
	{
		fprintf(stderr, " mysql_stmt_bind_result() failed\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(mPstmt));
		fprintf(stderr, " %ld\n", mysql_stmt_errno(mPstmt));
	}
}

void PrepareStatement::Excute()
{
	if (mysql_stmt_execute(mPstmt))
	{
		fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(mPstmt));
	}
}

int PrepareStatement::Fetch()
{
	if (mAffectRowCnt > 0)
	{
		return mysql_stmt_fetch(mPstmt);
	}
	return 1;	//fetch 실패
}

int PrepareStatement::GetResponse()
{
	switch (mQueryType)
	{
	case 0:
	{
		return GetOneResponse();
	}
	default:
		break;
	}
	return 0;
}

void PrepareStatement::ReadyToResponse()
{
	mAffectRowCnt = mysql_stmt_field_count(mPstmt);
	if (mAffectRowCnt > 0)
	{
		/* Now buffer all results to client (optional step) */
		// fetch할때 서버에서 하나씩 받아오는게 아닌, 한방에 버퍼에 넣어두기 위한 작업
		// 결과 집합을 생성하지 않는 SQL 문을 실행한 후 호출할 필요는 없지만 호출하더라도 성능에 해를 끼치거나 눈에 띄는 성능 문제가 발생하지는 않습니다. -mysql 공식문서
		if (mysql_stmt_store_result(mPstmt))
		{
			fprintf(stderr, " mysql_stmt_store_result() failed\n");
			fprintf(stderr, " %s\n", mysql_stmt_error(mPstmt));
			fprintf(stderr, " %ld\n", mysql_stmt_errno(mPstmt));

		}
	}
}

void PrepareStatement::SetQueryType()
{
	string tmp = "call";

	char buffer[5];
	strncpy_s(buffer, 5, mQuery.c_str(), 4);
	transform(cbegin(buffer), cend(buffer), begin(buffer), std::tolower);
	switch (tmp.compare(buffer))
	{
	case 0:
		mQueryType = 0;
		break;
	default:
		mQueryType = 1;
		break;
	}
}

int PrepareStatement::GetOneResponse()
{

	int status = 0;
	status = mysql_stmt_next_result(mPstmt);
	if (status > 0)
	{
		fprintf(stderr, " mysql_stmt_next_result() failed\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(mPstmt));
		fprintf(stderr, " %ld\n", mysql_stmt_errno(mPstmt));
		return status;
	}

	return status;
}


