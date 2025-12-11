#pragma once
/*
    实现Mysql数据库的增删改查
*/
#include <mysql/mysql.h>
#include <string>
using namespace std;
class Connection
{
public:
    /*初始化数据库连接*/
    Connection();
    /*释放数据库连接资源*/
    ~Connection();
    /*连接数据库*/
    bool connect(string ip,
        unsigned short port,
        string username,
        string password,
        string dbname);
    //更新操作 insert,delete,update
    bool update(string sql);
    //查询操作
    MYSQL_RES* query(string sql);
private:
    //表示跟MYSQL的一条连接
    MYSQL *_conn;
    
};