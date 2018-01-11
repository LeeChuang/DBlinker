#pragma once
#ifndef ODBC_DB_RECORD_H
#define ODBC_DB_RECORD_H

#include "ODBCDButil.h"

class CODBCDBRecord
{
private:
	SQLHANDLE m_handle_stmt;		//�����
	SQLRETURN m_ret_code;			//ִ��ODBC����Ľ��
	SQLINTEGER m_record_row_count;	//ִ��SQL��ѯ���¼����
	SQLINTEGER m_affect_row_count;	//ִ��SQL��Ӱ������
	SQLSMALLINT m_col_count;		//�ֶ�����
	
	/*
	��ѯ�����Ľ��
	map keyΪ�ֶ�����
	tuple��1������Ϊֵ���ͣ���2������Ϊֵ
	*/
	std::unordered_map<std::string, std::tuple<int, std::string>> m_query_data;
	//int m_data_index;				//��¼λ������

	bool SetQueryHandle(CODBCDButil* db_connection);
public:
	CODBCDBRecord();
	int ExecuteQuery(CODBCDButil* db_connection, const char* sql_data);				//ִ�в�ѯ���
	int ExecuteNonQuery(CODBCDButil* db_connection, const char* sql_data);			//ִ�зǲ�ѯ���
	~CODBCDBRecord();

	//inline void MoveFirst(){ m_data_index = 0; };
	bool MoveNext(); 
	//inline bool IsEnd(){ return m_data_index >= m_record_row_count; };
	inline int GetRecodeCount(){ return m_record_row_count; };

	//�����ֶ��ֶ��� ��ȡ�ֶ�����
	int GetColData(const std::string col_name, int data_type, void *col_data);
};

#endif