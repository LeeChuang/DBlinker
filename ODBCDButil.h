#pragma once
#ifndef ODBC_DB_UTIL_H
#define ODBC_DB_UTIL_H
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <mutex>
#include <vector>
#include <tuple>
#include <string>
#include <unordered_map>

//��ӦODBC����������
#define ODBC_SQL_UNKNOWN_TYPE    0
#define ODBC_SQL_CHAR            1
#define ODBC_SQL_NUMERIC         2
#define ODBC_SQL_DECIMAL         3
#define ODBC_SQL_INTEGER         4
#define ODBC_SQL_SMALLINT        5
#define ODBC_SQL_FLOAT           6
#define ODBC_SQL_REAL            7
#define ODBC_SQL_DOUBLE          8
#define ODBC_SQL_DATETIME        9
#define ODBC_SQL_VARCHAR         12

#define CONN_STR_OUT_LEN 256

class CODBCDButil
{
private:

	CODBCDButil();
	SQLHANDLE m_handle_env;			//�������
	SQLHANDLE m_handle_dbc;			//���Ӿ��
	SQLRETURN m_ret_code;			//ִ��ODBC����Ľ��

	bool m_is_connect;				//�Ƿ������
	static CODBCDButil *m_instance;	//����

public:
	inline SQLHANDLE GetHandleDBC(){ return m_handle_dbc; };
	void Close();
	bool Connect(const char* dsn_data);
	~CODBCDButil(){ DestroyInstance(); };
	static CODBCDButil * GetInstace();
	static void DestroyInstance()
	{
		if (m_instance != nullptr)
		{
			m_instance->Close();
			m_instance = nullptr;
		}
	}

};

#endif