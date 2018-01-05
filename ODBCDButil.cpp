#include "ODBCDButil.h"
#include <iostream>
#include <memory>

CODBCDButil *CODBCDButil::m_instance = nullptr;

CODBCDButil::CODBCDButil() :m_is_connect(false), m_record_row_count(0), m_data_index(0)
{
	//申请环境句柄
	m_ret_code = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &m_handle_env);	
	if ((m_ret_code != SQL_SUCCESS) && (m_ret_code != SQL_SUCCESS_WITH_INFO))
	{
		std::cout << "Erro AllocHandle" << m_ret_code << std::endl;
		return;
	}

	//设置环境
	m_ret_code = SQLSetEnvAttr(m_handle_env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
	if ((m_ret_code != SQL_SUCCESS) && (m_ret_code != SQL_SUCCESS_WITH_INFO))
	{
		std::cout << "Erro AllocHandle" << m_ret_code << std::endl;
		SQLFreeHandle(SQL_HANDLE_DBC, m_handle_env);
		return;
	}

	//申请连接句柄
	m_ret_code = SQLAllocHandle(SQL_HANDLE_DBC, m_handle_env, &m_handle_dbc);
	if ((m_ret_code != SQL_SUCCESS) && (m_ret_code != SQL_SUCCESS_WITH_INFO))
	{
		std::cout << "Erro AllocHandle" << m_ret_code << std::endl;
		SQLFreeHandle(SQL_HANDLE_DBC, m_handle_env);
		return;
	}

}

CODBCDButil * CODBCDButil::GetInstace(){
	static std::once_flag oc;	//用于call_once的局部静态变量
	std::call_once(oc, [&] {  m_instance = new CODBCDButil(); });
	return m_instance;
}

void CODBCDButil::Close()
{
	if (m_is_connect)
	{
		SQLDisconnect(m_handle_dbc);
		SQLFreeHandle(SQL_HANDLE_DBC, m_handle_dbc);
		SQLFreeHandle(SQL_HANDLE_ENV, m_handle_env);
		m_is_connect = false;
	}
}

CODBCDButil::~CODBCDButil()
{
}

bool CODBCDButil::Connect(const char* dsn_data)
{
	if (dsn_data == nullptr)
		return false;
	if (m_is_connect == true)
		return true;
	try
	{
		SQLTCHAR str_out_conn[CONN_STR_OUT_LEN] = { 0 };
		short out_conn = 0;

		m_ret_code = SQLDriverConnect(m_handle_dbc, NULL, (SQLTCHAR*)dsn_data, 
			SQL_NTS, str_out_conn, CONN_STR_OUT_LEN, &out_conn, SQL_DRIVER_COMPLETE);
		if (m_ret_code != SQL_SUCCESS && m_ret_code != SQL_SUCCESS_WITH_INFO)
			return false;

		m_ret_code = SQLAllocHandle(SQL_HANDLE_STMT,m_handle_dbc,&m_handle_stmt);
		if (m_ret_code != SQL_SUCCESS && m_ret_code != SQL_SUCCESS_WITH_INFO)
			return false;
	}
	catch (...)
	{
		return false;
	}
	m_is_connect = true;
	return true;
}


int CODBCDButil::Execute(const char* dsn_data)
{
	if (dsn_data == nullptr)
		return -1;
	if (m_is_connect == false)
		return -2;

	m_ret_code = SQLExecDirect(m_handle_stmt, (SQLCHAR*)dsn_data, SQL_NTS);
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
		std::unordered_map<std::string,std::tuple<int,std::string>> temp_data;
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
		//	std::string str_temp();
		//	std::cout << temp_buf.use_count();
		//	memcpy(&str_temp, temp_buf, sizeof(temp_buf)/sizeof(char));
			temp_data[col_name] = std::make_tuple<>(col_type, temp_buf);
		}

		m_query_data[m_record_row_count] = temp_data;
		m_record_row_count++;
	}

	//std::cout << std::get<1>(m_query_data[0].begin()->second) << std::endl;
	//数据查询出来后  置位索引位置
	m_data_index = 0;
	SQLCancel(m_handle_stmt);
	return m_record_row_count;
}

int CODBCDButil::GetColData(const std::string col_name, int data_type, void* col_data)
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
	//std::cout << str_col_data.c_str() << std::endl;
	return 0;
}