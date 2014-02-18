#include "odbchandler.h"
#include "stdio.h"
#include<iostream>
#include "tglobal.h"

ODBCHandler* ODBCHandler::m_instance = 0;
static int connected = 0;

static SQLHENV ODBC_env = SQL_NULL_HANDLE;
static SQLHDBC ODBC_con;


ODBCHandler* ODBCHandler::instance(const char *datasource)
{
    if(!m_instance){
        m_instance = new ODBCHandler(datasource);
    }
   return m_instance;
}


ODBCHandler::ODBCHandler(const char *datasource)
{
    urlCon = datasource;
    if(datasource != 0 )
    {
        fprintf(stderr,"Init datasource %s",datasource);
        odbc_init(datasource);
    }
}

ODBCHandler::~ODBCHandler(){
    odbc_disconnect();
}

int ODBCHandler::odbc_init(const char *conn_string)
{
    fprintf(stderr, "BEGIN\n");
    SQLRETURN ret;
   try
    {
        SQLCHAR outstr[1024];
        SQLSMALLINT outstrlen;

        fprintf(stderr, "1\n");
        if (ODBC_env == SQL_NULL_HANDLE || connected == 0)
        {
            ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &ODBC_env);
            if ((SQL_SUCCEEDED(ret) != SQL_SUCCESS) && (SQL_SUCCEEDED(ret) != SQL_SUCCESS_WITH_INFO)) {
                    fprintf(stderr, "odbc: Error AllocHandle\n");
                    connected = 0;
                    return -1;
            }

            fprintf(stderr, "2\n");
            ret = SQLSetEnvAttr(ODBC_env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);

            if ((SQL_SUCCEEDED(ret) != SQL_SUCCESS) && (SQL_SUCCEEDED(ret) != SQL_SUCCESS_WITH_INFO)) {
                    fprintf(stderr, "odbc: Error SetEnv\n");
                    SQLFreeHandle(SQL_HANDLE_ENV, ODBC_env);
                    ODBC_env = SQL_NULL_HANDLE;
                    connected = 0;
                    return -1;
            }

            fprintf(stderr, "3\n");
            ret = SQLAllocHandle(SQL_HANDLE_DBC, ODBC_env, &ODBC_con);

            if ((SQL_SUCCEEDED(ret) != SQL_SUCCESS) && (SQL_SUCCEEDED(ret) != SQL_SUCCESS_WITH_INFO)) {
                    fprintf(stderr, "odbc: Error AllocHDB %d\n", ret);
                    SQLFreeHandle(SQL_HANDLE_ENV, ODBC_env);
                    ODBC_env = SQL_NULL_HANDLE;
                    connected = 0;
                    return -1;
            }

            fprintf(stderr, "4\n");
           /* ret = SQLSetConnectAttr(ODBC_con, SQL_LOGIN_TIMEOUT, (SQLPOINTER *)10, 0);

            if ((SQL_SUCCEEDED(ret) != SQL_SUCCESS) && (SQL_SUCCEEDED(ret) != SQL_SUCCESS_WITH_INFO)) {
                    fprintf(stderr, "odbc: Error set Login timeout %d\n", ret);
                    SQLFreeHandle(SQL_HANDLE_ENV, ODBC_env);
                    ODBC_env = SQL_NULL_HANDLE;
                    connected = 0;
                    return -1;
            }
            fprintf(stderr, "5\n");
            */
        }

        fprintf(stderr, "6\n");
        ret = SQLDriverConnect(ODBC_con, NULL, (SQLCHAR *)conn_string, SQL_NTS, outstr, sizeof( outstr), &outstrlen, SQL_DRIVER_COMPLETE);

        fprintf(stderr, "6.1\n");
        if (SQL_SUCCEEDED(ret) == SQL_SUCCESS_WITH_INFO) {
                printf("Driver reported the following diagnostics\n");
                extract_error("SQLDriverConnect", ODBC_con, SQL_HANDLE_DBC);
        }

        fprintf(stderr, "7\n");
        if ((SQL_SUCCEEDED(ret) != SQL_SUCCESS) && (SQL_SUCCEEDED(ret) != SQL_SUCCESS_WITH_INFO)) {
                fprintf(stderr, "odbc: Error SQLConnect %d\n", ret);
                extract_error("SQLDriverConnect", ODBC_con, SQL_HANDLE_DBC);
                odbc_disconnect();
                return -1;
        } else {
                connected = 1;
        }

        fprintf(stderr, "8\n");
    }catch(...){
        fprintf(stderr, "odbc: Error SQLConnect %d\n", ret);
    }
    fprintf(stderr, "END\n");
    return 0;
}

void ODBCHandler::odbc_disconnect()
{
    SQLDisconnect(ODBC_con);
    SQLFreeHandle(SQL_HANDLE_DBC, ODBC_con);
    ODBC_con = SQL_NULL_HANDLE;
    SQLFreeHandle(SQL_HANDLE_ENV, ODBC_env);
    ODBC_env = SQL_NULL_HANDLE;
    connected = 0;
}

bool ODBCHandler::connect(const char *dsn)
{
     SQLHENV env;
     SQLHDBC dbc;
     SQLHSTMT stmt;
     SQLRETURN ret; /* ODBC API return status */
     SQLCHAR outstr[1024];
     SQLSMALLINT outstrlen;
     /* Allocate an environment handle */
     SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
     /* We want ODBC 3 support */
     SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);
     //SQLSetEnvAttr(henv_,     SQL_ATTR_CONNECTION_POOLING,     (SQLPOINTER)SQL_CP_ONE_PER_HENV,
     //SQL_IS_UINTEGER);    (or)     SQL_CP_ONE_PER_DRIVER
     /* Allocate a connection handle */
     SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
     /* Connect to the DSN mydsn */
     ret = SQLDriverConnect(dbc, NULL,(SQLCHAR*) dsn, SQL_NTS,
                outstr, sizeof(outstr), &outstrlen,
                SQL_DRIVER_COMPLETE);
     if (SQL_SUCCEEDED(ret)) {
       tDebug("Connected\n");
       tDebug("Returned connection string was:\n\t%s\n", outstr);
       if (ret == SQL_SUCCESS_WITH_INFO) {
         tDebug("Driver reported the following diagnostics\n");
         extract_error("SQLDriverConnect", dbc, SQL_HANDLE_DBC);
       }
       SQLDisconnect(dbc);		/* disconnect from driver */
     } else {
       tDebug( "Failed to connect\n");
       extract_error("SQLDriverConnect", dbc, SQL_HANDLE_DBC);
     }
     /* free up allocated handles */
     SQLFreeHandle(SQL_HANDLE_DBC, dbc);
     SQLFreeHandle(SQL_HANDLE_ENV, env);
     return SQL_SUCCEEDED(ret);
}

SQLHDBC ODBCHandler::getConnection(const char *dsn)
{
    //TODO:
}

void ODBCHandler::extract_error(char *fn,SQLHANDLE handle,SQLSMALLINT type)
{
    SQLSMALLINT	 i = 0;
    SQLINTEGER	 native;
    SQLCHAR	 state[ 7 ];
    SQLCHAR	 text[256];
    SQLSMALLINT	 len;
    SQLRETURN	 ret;

    fprintf(stderr,"\nThe driver reported the following diagnostics whilst running s\n\n",fn);

    do
    {
        ret = SQLGetDiagRec(type, handle, i, state, &native, text,sizeof(text), &len );
        if (SQL_SUCCEEDED(ret)){
            fprintf(stderr,"%s:%ld:%ld:%s\n", state, i, native, text);
        }else{
            fprintf(stderr,"Failed to get diagnostics :S %s:%ld:%ld:%s\n", state, i, native, text);
        }
        i++;
    }while( ret == SQL_SUCCESS );
}

void ODBCHandler::listDataSources()
{
    SQLHENV env;
    SQLCHAR dsn[256];
    SQLCHAR desc[256];
    SQLSMALLINT dsn_ret;
    SQLSMALLINT desc_ret;
    SQLUSMALLINT direction;
    SQLRETURN ret;

    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);

    direction = SQL_FETCH_FIRST;
    while(SQL_SUCCEEDED(ret = SQLDataSources(env, direction,
                         dsn, sizeof(dsn), &dsn_ret,
                         desc, sizeof(desc), &desc_ret))) {
      direction = SQL_FETCH_NEXT;
      tDebug("%s - %s\n", dsn, desc);
      if (ret == SQL_SUCCESS_WITH_INFO) tDebug("\tdata truncation\n");
    }
}

void ODBCHandler::listDrivers()
{
     SQLHENV env;
     SQLCHAR driver[256];
     //char attr[256];
     SQLCHAR attr[256];
     SQLSMALLINT driver_ret;
     SQLSMALLINT attr_ret;
     SQLUSMALLINT direction;
     SQLRETURN ret;

     SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
     SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);

     direction = SQL_FETCH_FIRST;
     while(SQL_SUCCEEDED(ret = SQLDrivers(env, direction,
                          driver, sizeof(driver), &driver_ret,
                          attr, sizeof(attr), &attr_ret))) {
       direction = SQL_FETCH_NEXT;
       tDebug("%s - %s\n", driver, attr);
       if (ret == SQL_SUCCESS_WITH_INFO) tDebug("\tdata truncation\n");
     }
}

void ODBCHandler::showTables(const char *dsn)
{
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    SQLRETURN ret; /* ODBC API return status */
    SQLSMALLINT columns; /* number of columns in result-set */
    int row = 0;

    /* Allocate an environment handle */
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    /* We want ODBC 3 support */
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3_80, 0);
    /* Allocate a connection handle */
    SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    /* Connect to the DSN mydsn */
    /* You will need to change mydsn to one you have created and tested */
    SQLDriverConnect(dbc, NULL, (SQLCHAR*) dsn, SQL_NTS,
                     NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    /* Allocate a statement handle */
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    /* Retrieve a list of tables */
    SQLTables(stmt, NULL, 0, NULL, 0, NULL, 0, (SQLCHAR*)"VIEW", SQL_NTS);
    /* How many columns are there */
    SQLNumResultCols(stmt, &columns);
    /* Loop through the rows in the result-set */
    while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
        SQLUSMALLINT i;
        printf("Row %d\n", row++);
        /* Loop through the columns */
        for (i = 1; i <= columns; i++) {
            SQLLEN indicator;
            SQLCHAR buf[512];
            /* retrieve column data as a string */
        ret = SQLGetData(stmt, i, SQL_C_CHAR,
                             buf, sizeof(buf), &indicator);
            if (SQL_SUCCEEDED(ret)) {
                /* Handle null columns */
                if (indicator == SQL_NULL_DATA) strcpy((char*)buf, "NULL");
            printf("  Column %u : %s\n", i, buf);
            }
        }
      }
}

void ODBCHandler::describeParameter(const char* sqlProc,int parameterIndex)
{
    SQLHSTMT stmt;
    SQLRETURN ret;
    SQLRETURN ret2;
    SQLSMALLINT columns; /* number of columns in result-set */
    QVariantMap rMap;
    int row = 0;

    if (ODBC_env != SQL_NULL_HANDLE || connected != 0)
    {
        SQLCHAR       Statement[100];
        SQLSMALLINT   NumParams, i, DataType, DecimalDigits, Nullable;
        SQLULEN   ParamSize;
        SQLHSTMT      hstmt;

        // Prompt the user for an SQL statement and prepare it.
        //GetSQLStatement(Statement);
       // SQLPrepare(hstmt, sqlProc, SQL_NTS);

        // Check to see if there are any parameters. If so, process them.
        SQLNumParams(hstmt, &NumParams);
        if (NumParams) {
           // Allocate memory for three arrays. The first holds pointers to buffers in which
           // each parameter value will be stored in character form. The second contains the
           // length of each buffer. The third contains the length/indicator value for each
           // parameter.
           //SQLPOINTER * PtrArray = (SQLPOINTER *) malloc(NumParams * sizeof(SQLPOINTER));
          // SQLINTEGER * BufferLenArray = (SQLINTEGER *) malloc(NumParams * sizeof(SQLINTEGER));
           //SQLINTEGER * LenOrIndArray = (SQLINTEGER *) malloc(NumParams * sizeof(SQLINTEGER));

           for (i = 0; i < NumParams; i++) {
           // Describe the parameter.
           SQLDescribeParam(hstmt, i + 1, &DataType, &ParamSize, &DecimalDigits, &Nullable);

           // Call a helper function to allocate a buffer in which to store the parameter
           // value in character form. The function determines the size of the buffer from
           // the SQL data type and parameter size returned by SQLDescribeParam and returns
           // a pointer to the buffer and the length of the buffer.
           //AllocParamBuffer(DataType, ParamSize, &PtrArray[i], &BufferLenArray[i]);

           // Bind the memory to the parameter. Assume that we only have input parameters.
          /* SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT, SQL_C_CHAR, DataType, ParamSize,
                 DecimalDigits, PtrArray[i], BufferLenArray[i],
                 &LenOrIndArray[i]);
           */

           // Prompt the user for the value of the parameter and store it in the memory
           // allocated earlier. For simplicity, this function does not check the value
           // against the information returned by SQLDescribeParam. Instead, the driver does
           // this when the statement is executed.
           //GetParamValue(PtrArray[i], BufferLenArray[i], &LenOrIndArray[i]);
           }
        }

        // Execute the statement.
        //SQLExecute(hstmt);

        // Process the statement further, such as retrieving results (if any) and closing the
        // cursor (if any). Code not shown.

        // Free the memory allocated for each parameter and the memory allocated for the arrays
        // of pointers, buffer lengths, and length/indicator values.
        //for (i = 0; i < NumParams; i++) free(PtrArray[i]);
        //free(PtrArray);
        //free(BufferLenArray);
        //free(LenOrIndArray);
    }
}

bool ODBCHandler::allocHandle(SQLHSTMT *stmt)
{
    SQLRETURN ret;  /* ODBC API return status */

    ret = SQLAllocHandle(SQL_HANDLE_STMT, ODBC_con, stmt);

    if (!SQL_SUCCEEDED(ret)) {
            fprintf(stderr, "Failed to allocate stmt handle.\n");
            extract_error("SQLAllocHandle", ODBC_con, SQL_HANDLE_DBC);
            return false;
    }
    return true;
}

void ODBCHandler::printStatement(SQLHSTMT *stmt)
{
    SQLRETURN ret;
    SQLRETURN ret2;
    SQLSMALLINT columns; /* number of columns in result-set */
    int row = 0;


    /* How many columns are there */
    SQLNumResultCols(*stmt, &columns);

    /* Loop through the rows in the result-set */
    while (SQL_SUCCEEDED(ret = SQLFetch(*stmt))) {
        SQLUSMALLINT i;
        ColDesc c;
        fprintf(stderr,"Row %d\n", row++);
        /* Loop through the columns */
        for (i = 1; i <= columns; i++) {
            SQLLEN indicator;
            SQLCHAR buf[512];
            //get column info
            ret2 =   SQLDescribeCol(*stmt,i,c.colName, sizeof(c.colName), &c.nameLen,NULL, NULL, NULL, NULL);
            /* retrieve column data as a string */
            ret = SQLGetData(*stmt, i, SQL_C_CHAR,buf, sizeof(buf), &indicator);
            if (SQL_SUCCEEDED(ret)) {
                /* Handle null columns */
                if (indicator == SQL_NULL_DATA) strcpy((char*)buf, "NULL");
                fprintf(stderr,"  Column %s %u : %s\n", c.colName,i, buf);
            }
        }

      }
}

bool ODBCHandler::odbc_execute_query(SQLHSTMT *stmt, const char *sql)
{
    /* Call as: if (false == odbc_execute_query(&stmt, sql)) { goto SALIR; } */
    SQLRETURN ret;  /* ODBC API return status */

    ret = SQLAllocHandle(SQL_HANDLE_STMT, ODBC_con, stmt);

    if (!SQL_SUCCEEDED(ret)) {
            fprintf(stderr, "Failed to allocate stmt handle.\n");
            extract_error("SQLAllocHandle", ODBC_con, SQL_HANDLE_DBC);
            return false;
    }

    ret = SQLExecDirect(*stmt, (SQLCHAR *)sql, SQL_NTS);

    if (!SQL_SUCCEEDED(ret)) {
            fprintf(stderr, "Failed to execute SQLExecDirect [%s].\n", sql);
            extract_error("SQLExecDirect", ODBC_con, SQL_HANDLE_DBC);
            SQLFreeHandle(SQL_HANDLE_STMT, *stmt);
            return false;
    }

    return true;
}

bool ODBCHandler::odbc_exec_stmt(SQLHSTMT *stmt, const char *sql)
{
    SQLRETURN ret;  /* ODBC API return status */
    ret = SQLExecDirect(*stmt, (SQLCHAR *)sql, SQL_NTS);

    if (!SQL_SUCCEEDED(ret)) {
            fprintf(stderr, "Failed to execute SQLExecDirect [%s].\n", sql);
            extract_error("SQLExecDirect", stmt, SQL_HANDLE_STMT);
            //SQLFreeHandle(SQL_HANDLE_STMT, *stmt);//external alloc, so must be external dealloc
            return false;
    }
    return true;
}

QVariantMap ODBCHandler::ExecuteQuery(const char *sql, int *ok)
{

}

void ODBCHandler::ExecuteQuery(const char *sql, QVariantList &list, int *ok)
{
    SQLHSTMT stmt;
    SQLRETURN ret;
    SQLRETURN ret2;
    SQLSMALLINT columns; /* number of columns in result-set */
    QVariantMap rMap;
    int row = 0;

    if (ODBC_env != SQL_NULL_HANDLE || connected != 0)
    {
        if(odbc_execute_query(&stmt,sql))
        {
            /* How many columns are there */
            SQLNumResultCols(stmt, &columns);

            /* Loop through the rows in the result-set */
            while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
                SQLUSMALLINT i;
                ColDesc c;
                printf("Row %d\n", row++);
                /* Loop through the columns */
                for (i = 1; i <= columns; i++) {
                    SQLLEN indicator;
                    SQLCHAR buf[512];
                    //get column info
                    ret2 =   SQLDescribeCol(stmt,i,c.colName, sizeof(c.colName), &c.nameLen,NULL, NULL, NULL, NULL);
                    /* retrieve column data as a string */
                    ret = SQLGetData(stmt, i, SQL_C_CHAR,buf, sizeof(buf), &indicator);
                    if (SQL_SUCCEEDED(ret) && SQL_SUCCEEDED(ret2)) {
                        /* Handle null columns */
                        if (indicator == SQL_NULL_DATA) strcpy((char*)buf, "NULL");
                        //printf("  Column %s %u : %s\n", c.colName,i, buf);
                        rMap.insert(QString( reinterpret_cast<char*>(c.colName)),QString( reinterpret_cast<char*>(buf)));
                    }
                }
                list.push_back(rMap);
              }
        }else{
            //TODO:validate on error
        }
    }
}

void ODBCHandler::testQuery(const char *query)
{
    SQLHSTMT stmt;
    SQLRETURN ret;
    SQLRETURN ret2;
    SQLSMALLINT columns; /* number of columns in result-set */
    int row = 0;

    if (ODBC_env != SQL_NULL_HANDLE || connected != 0)
    {
        if(odbc_execute_query(&stmt,query))
        {
            /* How many columns are there */
            SQLNumResultCols(stmt, &columns);
            /* Loop through the rows in the result-set */
            while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
                SQLUSMALLINT i;
                ColDesc c;
                printf("Row %d\n", row++);
                /* Loop through the columns */
                for (i = 1; i <= columns; i++) {
                    SQLLEN indicator;
                    SQLCHAR buf[512];
                    /* retrieve column data as a string */
                    ret2 =   SQLDescribeCol(stmt,i,
                                                          c.colName, sizeof(c.colName), &c.nameLen,
                                                          &c.dataType, &c.colSize, &c.decimalDigits, &c.nullable);

                    ret = SQLGetData(stmt, i, SQL_C_CHAR,buf, sizeof(buf), &indicator);
                    if (SQL_SUCCEEDED(ret)) {
                        /* Handle null columns */
                        if (indicator == SQL_NULL_DATA) strcpy((char*)buf, "NULL");
                        printf("  Column %s %u : %s\n", c.colName,i, buf);
                    }
                }
              }
        }else{

        }
    }
}

void ODBCHandler::testQuery(const char* query, const char* dsn)
{
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    SQLRETURN ret; /* ODBC API return status */
    SQLRETURN ret2; /* ODBC API return status */
    SQLSMALLINT columns; /* number of columns in result-set */
    int row = 0;

    /* Allocate an environment handle */
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    /* We want ODBC 3 support */
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);
    /* Allocate a connection handle */
    SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    /* Connect to the DSN mydsn */
    /* You will need to change mydsn to one you have created and tested */
    SQLDriverConnect(dbc, NULL, (SQLCHAR*) dsn, SQL_NTS,
                     NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    /* Allocate a statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

    if (!SQL_SUCCEEDED(ret)) {
            fprintf(stderr, "Failed to allocate stmt handle.\n");
            extract_error("SQLAllocHandle", ODBC_con, SQL_HANDLE_DBC);
    }

    ret = SQLExecDirect(stmt, (SQLCHAR *)query, SQL_NTS);

    if (!SQL_SUCCEEDED(ret)) {
            fprintf(stderr, "Failed to execute SQLExecDirect [%s].\n", query);
            extract_error("SQLExecDirect", ODBC_con, SQL_HANDLE_DBC);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    /* How many columns are there */
    SQLNumResultCols(stmt, &columns);
    /* Loop through the rows in the result-set */
    while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
        SQLUSMALLINT i;
        ColDesc c;
        printf("Row %d\n", row++);
        /* Loop through the columns */
        for (i = 1; i <= columns; i++) {
            SQLLEN indicator;
            SQLCHAR buf[512];
            /* retrieve column data as a string */
            ret2 =   SQLDescribeCol(stmt,i,
                                                  c.colName, sizeof(c.colName), &c.nameLen,
                                                  &c.dataType, &c.colSize, &c.decimalDigits, &c.nullable);

            ret = SQLGetData(stmt, i, SQL_C_CHAR,buf, sizeof(buf), &indicator);
            if (SQL_SUCCEEDED(ret)) {
                /* Handle null columns */
                if (indicator == SQL_NULL_DATA) strcpy((char*)buf, "NULL");
                printf("  Column %s %u : %s\n", c.colName,i, buf);
            }
        }
      }
}
