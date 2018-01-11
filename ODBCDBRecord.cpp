#include "ODBCDBRecord.h"
#include "glog/logging.h"

CODBCDBRecord::CODBCDBRecord()
:m_record_row_count(0), 
m_col_count(0), 
//m_data_index(0), 
m_affect_row_count(0)
{
	//LOG(INFO) << "init ODBCRecord.";
}


CODBCDBRecord::~CODBCDBRecord()
{
	//LOG(INFO) << "delete ODBCRecord.";
	SQLFreeHandle(SQL_HANDLE_STMT, m_handle_stmt);
	if (m_query_data.size() > 1)
		m_query_data.clear();
}

int CODBCDBRecord::ExecuteNonQuery(CODBCDButil* db_connection, const char* sql_data)
{
	//LOG(INFO) << "ExecuteNonQuery.";
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
bool CODBCDBRecord::MoveNext()
{
	SQLRETURN sql_ret = SQLFetch(m_handle_stmt);
	if (sql_ret == SQL_NO_DATA)
	{
		SQLCancel(m_handle_stmt);
		return false;
	}

	SQLLEN col_len = 0;
	SQLSMALLINT buf_len = 0;
	SQLLEN col_type = 0;

	try
	{
		for (int i = 1; i <= m_col_count; ++i)
		{
			char * temp_buf = nullptr;
			char col_name[256] = { 0 };
			SQLColAttribute(m_handle_stmt, i, SQL_DESC_NAME, col_name, 256, &buf_len, 0);
			SQLColAttribute(m_handle_stmt, i, SQL_DESC_TYPE, 0, 0, 0, &col_type);
			SQLColAttribute(m_handle_stmt, i, SQL_DESC_LENGTH, NULL, 0, 0, &col_len);

			temp_buf = new char[(int)col_len + 1];
			memset(temp_buf, 0, sizeof(temp_buf));
			SQLGetData(m_handle_stmt, i, SQL_C_CHAR, temp_buf, col_len + 1, NULL);

			m_query_data[col_name] = std::make_tuple((int)col_type, temp_buf);

			delete[] temp_buf;
		}
		m_record_row_count++;
	}
	catch (...)
	{
		return false;
	}
	return true;
	
}
int CODBCDBRecord::ExecuteQuery(CODBCDButil* db_connection, const char* sql_data)
{
	if (!SetQueryHandle(db_connection))
		return -6;

	if (sql_data == nullptr)
		return -1;
	try
	{
		//LOG(INFO) << "ExecuteQuery.";
		m_ret_code = SQLExecDirect(m_handle_stmt, (SQLCHAR*)sql_data, SQL_NTS);
		if (m_ret_code != SQL_SUCCESS && m_ret_code != SQL_SUCCESS_WITH_INFO)
			return -3;

		m_ret_code = SQLNumResultCols(m_handle_stmt, &m_col_count);
		if (m_ret_code != SQL_SUCCESS && m_ret_code != SQL_SUCCESS_WITH_INFO)
			return -4;

		m_record_row_count = 0;
	}
	catch (...)
	{
		return -5;
	}
	return m_record_row_count;

}

bool CODBCDBRecord::SetQueryHandle(CODBCDButil* db_connection)
{
	if ((db_connection == NULL) || (!db_connection->GetConnection()))
		return false;
	//LOG(INFO) << "Alloc Query Handle.";
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
	if (m_query_data.find(col_name) == m_query_data.end())
		return -1;		//数据中没有当前字段

	if (std::get<0>(m_query_data[col_name]) != data_type)
		return -3;		//获取的数据类型和当前字段的数据类型不一致

	std::string str_col_data = std::get<1>(m_query_data[col_name]);

	switch (data_type)
	{
	case ODBC_SQL_INTEGER:
		*((int*)col_data) = atoi(str_col_data.c_str());
		break;
	case ODBC_SQL_CHAR:
	case ODBC_SQL_VARCHAR:
	case ODBC_SQL_DATETIME:
		*((std::string*)col_data) = str_col_data;
		break;
	case ODBC_SQL_FLOAT:
		*((float*)col_data) = (float)atof(str_col_data.c_str());
		break;
	case ODBC_SQL_DOUBLE:
		*((double*)col_data) = atof(str_col_data.c_str());
		break;
	default:

		break;
	}

	//查询出来之后 将当前数据从map中删除
	m_query_data.erase(col_name);
	return 0;
}