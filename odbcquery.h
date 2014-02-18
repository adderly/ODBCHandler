#ifndef ODBCQUERY_H
#define ODBCQUERY_H

#include"odbchandler.h"
#include <stdio.h>

/**
*   Class to manage queries and parameter binding for unixodbc.
**/
class ODBCQuery
{
public:
    ODBCQuery();
    ~ODBCQuery();

    /**
    *   Prepares an stament from an active ODBCHandler connection.
    *   and set the sql string.
    *   return false if failed to prepare the statement.
    */
    bool prepare(const char* sql);

    /**
    *   Bing parameters to and statament.
    *   return false if failed to bind parameter.
    */
    bool  BindParameter(SQLUSMALLINT ipar,SQLSMALLINT  fParamType,
        SQLSMALLINT  fCType,SQLSMALLINT  fSqlType,SQLULEN cbColDef,
        SQLSMALLINT ibScale,SQLPOINTER rgbValue,
        SQLLEN cbValueMax,SQLLEN *pcbValue);

    /**
    *   execute the prepared statement.
    *   return false if failed to execute query.
    */
    bool exec();

    /**
    *
    */
    SQLHSTMT* getStatement(){return &_stmnt;}

private:
    bool allocated;
    SQLHSTMT _stmnt;
    ODBCHandler* dbhandler;
    const char* _query_str;
};

#endif // ODBCQUERY_H
