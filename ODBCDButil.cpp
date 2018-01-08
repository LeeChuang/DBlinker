#include "ODBCDButil.h"

CODBCDButil *CODBCDButil::m_instance = nullptr;

CODBCDButil::CODBCDButil() :m_is_connect(false)
{
	//申请环境句柄
	m_ret_code = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &m_handle_env);	
	if ((m_ret_code != SQL_SUCCESS) && (m_ret_code != SQL_SUCCESS_WITH_INFO))
		return;
	

	//设置环境
	m_ret_code = SQLSetEnvAttr(m_handle_env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
	if ((m_ret_code != SQL_SUCCESS) && (m_ret_code != SQL_SUCCESS_WITH_INFO))
		return;
	

	//申请连接句柄
	m_ret_code = SQLAllocHandle(SQL_HANDLE_DBC, m_handle_env, &m_handle_dbc);
	if ((m_ret_code != SQL_SUCCESS) && (m_ret_code != SQL_SUCCESS_WITH_INFO))
		return;
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
	}
	catch (...)
	{
		return false;
	}
	m_is_connect = true;
	return true;
}
