#include <mysql/mysql.h>
#include <iostream>
#include <string>

#ifndef DATABASE_H
#define DATABASE_H

using namespace std;

class Database {

    MYSQL conn;
    MYSQL_RES *qry;
    MYSQL_ROW row;
    MYSQL_FIELD *fields;

    public:
        bool connected;
        int open(string host,string username,string password,string database);
        int set (string sql);
        int get (string sql); // get para fazer a cosulta
        int get_num_rows();
        string get (string field,int line); // get pra pegar a consulta
        void clear ();
        string getError();
        void close ();
        const string sqlconv(string sql);
        Database();
        ~Database();
};

#endif

