#ifndef ODBCHANDLER_H
#define ODBCHANDLER_H

#include"sql.h"
#include"sqlext.h"

#ifdef QT_VERSION
#include<QVariant>
#endif


/**
*   Struct for store column info.
*/
struct ColDesc
{
    SQLSMALLINT colNumber;
    SQLCHAR colName[80];
    SQLSMALLINT nameLen;
    SQLSMALLINT dataType;
    SQLULEN colSize;
    SQLSMALLINT decimalDigits;
    SQLSMALLINT nullable;
};


/**
*   COMPILED WITH unixODBC[3.2.2]
*/
/**
*
*   This class will handle connections and Return of results from the database.
*   Simple wrapper to execute queries and get back a result.
*
*/
class ODBCHandler
{
  public:
    ~ODBCHandler();


    static ODBCHandler* instance(const char* datasource = 0);
    /**
    *   List the the drivers configured with odbc.
    *   Most likely the ones present in the defined odbcinst.ini
    */
    void listDrivers();

    /**
    *   List the datasources configured to odbc.
    *   This are present in the defined file odbc.ini.
    */
    void listDataSources();

    /**
    *   Check connectivity to a datasource defined.
    *   Ex:
    *       DSN=fred;
    *       DSN=fred;UID=userid;PWD=password;more=..;
    *
    */
    bool connect(const  char* dsn);

    /**
    *   Get A connection from a=-=-=
    *   TODO:
    */
    SQLHDBC getConnection(const char* dsn);

    /**
    *   Show tables available in the DSN.
    */
    void showTables(const char* dsn);

    /**
    *   Describe parameter of a store procedure.
    	TODO:
    */
    void describeParameter(const char* sqlProc,int parameterIndex);

    /**
    *   Get info for a returned error.
    */
    static void extract_error(char *fn,SQLHANDLE handle,SQLSMALLINT type);

    /**
    *   Initialize the connection string.
    */
    int odbc_init(const char *conn_string);

    /**
    *   Disconnect the ODBC handlers.
    *   Called from destructor.
    */
    void odbc_disconnect(void);

    /**
    *   Allocate a Statement, for external use.
    */
    bool allocHandle(SQLHSTMT *stmt);

    /**
    *   Print and valid  statement.
    */
    void printStatement(SQLHSTMT *stmt);

    /**
    *   Execute query.
    *   This method allocates the statement inside the function.
    */
    inline  bool odbc_execute_query(SQLHSTMT *stmt,const char *sql);

    /**
    *   Executes an already allocated statement.
    *   This one is handy in case of parameter binding.
    */
    bool odbc_exec_stmt(SQLHSTMT *stmt,const char *sql);

#ifdef  QT_VERSION
    /**
    *   Returns A map containing al the result.
    */
    QVariantMap ExecuteQuery(const char*  sql,int *ok = 0);

    /**
    *
    */
    void ExecuteQuery(const char *sql, QVariantList& list,int *ok);
#endif

    /**
    *   Testing purposes:
    *   It will execute a specified query and the connection string,
    *   if its ok it will write the returned data.
    */
    void testQuery(const char* query);

    /**
    *   Testing purposes:
    *   It will execute a specified query and the connection string,
    *   if its ok it will write the returned data.
    */
    void testQuery(const char* query,const char* dsn);

private:
    ODBCHandler(const char* datasource = 0);

     static ODBCHandler* m_instance;
    //handlers
     //static SQLHENV ODBC_env ;
     //static SQLHDBC ODBC_con;
     //static int connected ;
    const char* urlCon;//url that might have been provided in the constructor.
    const char* connectedUrl;//once connected t
};



#endif // ODBCHANDLER_H
