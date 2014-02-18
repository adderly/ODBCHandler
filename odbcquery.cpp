#include "odbcquery.h"

ODBCQuery::ODBCQuery()
{
    allocated = false;
}

ODBCQuery::~ODBCQuery()
{
    SQLFreeHandle(SQL_HANDLE_STMT, _stmnt);
}

bool ODBCQuery::prepare(const char* sql)
{
    _query_str = sql;
    dbhandler = ODBCHandler::instance();
    if(dbhandler){
        allocated = dbhandler->allocHandle(&_stmnt);
    }
    return allocated;
}

bool ODBCQuery::BindParameter(SQLUSMALLINT ipar, SQLSMALLINT fParamType,
                              SQLSMALLINT fCType, SQLSMALLINT fSqlType, SQLULEN cbColDef,
                              SQLSMALLINT ibScale, SQLPOINTER rgbValue, SQLLEN cbValueMax,
                              SQLLEN *pcbValue)
{
    SQLRETURN ret;
    if(!allocated)
        return false;
    ret = SQLBindParameter(_stmnt,ipar,fParamType,fCType,fSqlType,cbColDef,
                ibScale,rgbValue,cbValueMax,pcbValue);

    return SQL_SUCCEEDED(ret);
}

bool ODBCQuery::exec()
{
    bool ret = false;
    //SQLBindParameter()
    if(!allocated && _query_str != 0) {
         return false;
    }else{
        fprintf(stderr, "odbc: Start query exec\n");
       ret = dbhandler->odbc_exec_stmt(&_stmnt,_query_str);
       if(ret) dbhandler->printStatement(&_stmnt);
       fprintf(stderr, "odbc: end query exec\n");
    }
    return ret;
}


