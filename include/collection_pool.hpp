#pragma once
#include <string>
#include <queue>
#include <mutex>
using namespace std;
#include <collection.hpp>
/*
    Connection Pool
*/
class ConnectionPool
{
    public:
        //获取连接池对象实例
        static ConnectionPool* getConnectionPool(); 
        bool loadConfigFile();  //从conf文件中加载配置项
    private:
        ConnectionPool();  //单例设计模式，构造函数私有化
        
        //mysql登录相关内容
        string _ip;
        unsigned short _port;
        string _username;
        string _password;
        //连接库相关数据
        int _initSize;         //连接池的初始连接量
        int _maxSize;          //连接池的最大连接量
        int _maxIdleTime;      //连接池的最大空闲时间
        int _connectionTimeout;//连接池获取连接的超时时间

        queue<Connection*> _connectionQue; //存储mysql连接的队列
        mutex _queueMutex; //维护连接队列的线程安全互斥锁
};