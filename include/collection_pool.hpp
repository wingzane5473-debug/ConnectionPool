#pragma once

/*
    Connection Pool
*/
class ConnectionPool
{
    public:
        //获取连接池对象实例
        static ConnectionPool* getConnectionPool(); 
    private:
        ConnectionPool();  //单例设计模式，构造函数私有化
};