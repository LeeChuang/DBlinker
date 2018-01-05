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
	SQLHANDLE m_handle_stmt;		//�����
	SQLRETURN m_ret_code;			//ִ��ODBC����Ľ��
	SQLINTEGER m_record_row_count;	//ִ��SQL��ѯ���¼����
	SQLINTEGER m_affect_row_count;	//ִ��SQL��Ӱ������
	SQLSMALLINT m_col_count;		//�ֶ�����

	bool m_is_connect;				//�Ƿ������
	static CODBCDButil *m_instance;	//����
	/*
	��ѯ�����Ľ��
	map keyΪ�ֶ�����
	tuple��1������Ϊֵ���ͣ���2������Ϊֵ
	*/
	std::unordered_map<int,std::unordered_map<std::string, std::tuple<int, std::string>>> m_query_data;
	int m_data_index;				//��¼λ������

public:

	void Close();
	bool Connect(const char* dsn_data);
	int Execute(const char* dsn_data);
	~CODBCDButil();
	

	inline void MoveFirst(){ m_data_index = 0; };
	inline void MoveNext(){ m_data_index++; };
	inline bool IsEnd(){ return m_data_index >= m_record_row_count; };
	int GetColData(const std::string col_name, int data_type, void *col_data);

	static CODBCDButil * GetInstace();
	static void DestroyInstance()
	{
		if (m_instance != nullptr)
		{
			m_instance->Close();
			delete m_instance;
			m_instance = nullptr;
		}
	}

};

#endif