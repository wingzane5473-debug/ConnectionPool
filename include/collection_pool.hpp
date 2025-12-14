#pragma once
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
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
       //给外部提供接口，从连接池中获取一个可用的空闲连接
        Connection* getConnection();
    private:
        ConnectionPool();  //单例设计模式，构造函数私有化
        bool loadConfigFile();  //从conf文件中加载配置项
        //mysql登录相关内容
        string _ip;
        unsigned short _port;
        string _dbname;    
        string _username;
        string _password;
        //连接库相关数据
        int _initSize;         //连接池的初始连接量
        int _maxSize;          //连接池的最大连接量
        int _maxIdleTime;      //连接池的最大空闲时间
        int _connectionTimeout;//连接池获取连接的超时时间

        queue<Connection*> _connectionQue; //存储mysql连接的队列
        mutex _queueMutex; //维护连接队列的线程安全互斥锁
        atomic_int _connectionCnt;  //记录连接所创建的connection连接的总数量
    };