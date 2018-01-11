#pragma once
#ifndef ODBC_DB_RECORD_H
#define ODBC_DB_RECORD_H

#include "ODBCDButil.h"

class CODBCDBRecord
{
private:
	SQLHANDLE m_handle_stmt;		//语句句柄
	SQLRETURN m_ret_code;			//执行ODBC语句后的结果
	SQLINTEGER m_record_row_count;	//执行SQL查询后记录条数
	SQLINTEGER m_affect_row_count;	//执行SQL后影响条数
	SQLSMALLINT m_col_count;		//字段总数
	
	/*
	查询出来的结果
	map key为字段名称
	tuple第1个参数为值类型，第2个参数为值
	*/
	std::unordered_map<std::string, std::tuple<int, std::string>> m_query_data;
	//int m_data_index;				//记录位置索引

	bool SetQueryHandle(CODBCDButil* db_connection);
public:
	CODBCDBRecord();
	int ExecuteQuery(CODBCDButil* db_connection, const char* sql_data);				//执行查询语句
	int ExecuteNonQuery(CODBCDButil* db_connection, const char* sql_data);			//执行非查询语句
	~CODBCDBRecord();

	//inline void MoveFirst(){ m_data_index = 0; };
	bool MoveNext(); 
	//inline bool IsEnd(){ return m_data_index >= m_record_row_count; };
	inline int GetRecodeCount(){ return m_record_row_count; };

	//根据字段字段名 获取字段数据
	int GetColData(const std::string col_name, int data_type, void *col_data);
};

#endif