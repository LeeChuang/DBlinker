#include "ODBCDBRecord.h"


CODBCDBRecord::CODBCDBRecord()
:m_record_row_count(0), 
m_col_count(0), 
m_data_index(0), 
m_affect_row_count(0)
{

}


CODBCDBRecord::~CODBCDBRecord()
{
	SQLFreeHandle(SQL_HANDLE_STMT, m_handle_stmt);
	if (m_query_data.size() > 1)
		m_query_data.clear();
}

int CODBCDBRecord::ExecuteNonQuery(CODBCDButil* db_connection, const char* sql_data)
{
	if (!SetQueryHandle(db_connection))
		return -6;

	m_affect_row_count = 0;
	if (sql_data == nullptr)
		return -1;
	try
	{
		m_ret_code = SQLExecDirect(m_handle_stmt, (SQLCHAR*)sql_data, SQL_NTS);
		if ((m_ret_code != SQL_SUCCESS) && (m_ret_code != SQL_SUCCESS_WITH_INFO))
			return -3;

		m_ret_code = SQLRowCount(m_handle_stmt, (SQLLEN*)&m_affect_row_count);
		if ((m_ret_code != SQL_SUCCESS) && (m_ret_code != SQL_SUCCESS_WITH_INFO))
			return -4;

		SQLCancel(m_handle_stmt);

	}
	catch (...)
	{
		return -5;
	}

	return m_affect_row_count;
}

int CODBCDBRecord::ExecuteQuery(CODBCDButil* db_connection, const char* sql_data)
{
	if (!SetQueryHandle(db_connection))
		return -6;

	if (sql_data == nullptr)
		return -1;
	try
	{
		m_ret_code = SQLExecDirect(m_handle_stmt, (SQLCHAR*)sql_data, SQL_NTS);
		if (m_ret_code != SQL_SUCCESS && m_ret_code != SQL_SUCCESS_WITH_INFO)
			return -3;

		m_ret_code = SQLNumResultCols(m_handle_stmt, &m_col_count);
		if (m_ret_code != SQL_SUCCESS && m_ret_code != SQL_SUCCESS_WITH_INFO)
			return -4;

		m_record_row_count = 0;

		while (true)
		{
			SQLRETURN sql_ret = SQLFetch(m_handle_stmt);
			if (sql_ret == SQL_NO_DATA)
				break;
			std::unordered_map<std::string, std::tuple<int, std::string>> temp_map;
			for (int i = 1; i <= m_col_count; ++i)
			{
				SQLLEN col_len = 0;
				SQLSMALLINT buf_len = 0;
				SQLLEN col_type = 0;
				char col_name[256] = { 0 };
				
				SQLColAttribute(m_handle_stmt, i, SQL_DESC_NAME, col_name, 256, &buf_len, 0);
				SQLColAttribute(m_handle_stmt, i, SQL_DESC_TYPE, 0, 0, 0, &col_type);
				SQLColAttribute(m_handle_stmt, i, SQL_DESC_LENGTH, NULL, 0, 0, &col_len);

				char * temp_buf = new char(col_len + 1);
				SQLGetData(m_handle_stmt, i, SQL_C_CHAR, temp_buf, 256, NULL);
				temp_map[col_name] = std::make_tuple<>(col_type, temp_buf);

			}

			m_query_data[m_record_row_count] = temp_map;
			m_record_row_count++;
		}
		//数据查询出来后  置位索引位置
		m_data_index = 0;
		SQLCancel(m_handle_stmt);
	}
	catch (...)
	{
		return -5;
	}
	return m_record_row_count;

}

bool CODBCDBRecord::SetQueryHandle(CODBCDButil* db_connection)
{
	if (db_connection == NULL)
		return false;

	try
	{
		m_ret_code = SQLAllocHandle	(SQL_HANDLE_STMT, db_connection->GetHandleDBC(), &m_handle_stmt);
		if (m_ret_code != SQL_SUCCESS && m_ret_code != SQL_SUCCESS_WITH_INFO)
			return false;
	}
	catch (...)
	{
		return false;
	}

	return true;
}

int CODBCDBRecord::GetColData(const std::string col_name, int data_type, void *col_data)
{
	if (m_query_data.find(m_data_index) == m_query_data.end())
		return -1;		//当前索引无数据

	if (m_query_data[m_data_index].find(col_name) == m_query_data[m_data_index].end())
		return -2;		//数据中没有当前字段

	if (std::get<0>(m_query_data[m_data_index][col_name]) != data_type)
		return -3;		//获取的数据类型和当前字段的数据类型不一致

	std::string str_col_data = std::get<1>(m_query_data[m_data_index][col_name]);

	switch (data_type)
	{
	case ODBC_SQL_INTEGER:
		*((int*)col_data) = atoi(str_col_data.c_str());
		break;
	case ODBC_SQL_CHAR:
		*((std::string*)col_data) = str_col_data;
		break;
	default:

		break;
	}

	m_query_data[m_data_index].erase(col_name);
	return 0;
}