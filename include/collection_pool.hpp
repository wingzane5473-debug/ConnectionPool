#pragma once
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>
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
       //给外部提供接口，从连接池中获取一个可用的空闲连接。
       //通过智能指针，直接释放掉connection，不需要再使用backConnection()函数
        shared_ptr<Connection> getConnection();
    private:
        ConnectionPool();  //单例设计模式，构造函数私有化
        bool loadConfigFile();  //从conf文件中加载配置项
        //运行在独立的线程中，专门负责生产新连接
        void produceConnectionTask();

        //扫描超过_maxIdleTime时间的空闲连接，进行连接的回收
        void scannerConnectionTask();
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
        condition_variable cv; //条件变量，用于生产者和消费者线程之间的通信
    };