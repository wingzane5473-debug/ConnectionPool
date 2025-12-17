#include <string>
#include "collection_pool.hpp"
#include "public.h"
//线程安全的懒汉单例模式
ConnectionPool* ConnectionPool::getConnectionPool()
{
    static ConnectionPool pool; //静态对象只初始化一次。自动进行lock与unlock
    return &pool;
}

bool ConnectionPool::loadConfigFile()
{
    FILE *pf = fopen(CONFIG_PATH,"r");
    if(pf == nullptr)
    {
        LOG("mysql.conf file is not exist!");
        return false;
    }
    while(!feof(pf))
    {
        char line[1024] = {0};
        fgets(line,1024,pf);
        string str = line;

        //找到每行的mapping
        int idx = str.find('=',0);
        if(idx==-1) //无效配置项
        {
            continue;
        }
        int endidx = str.find('\n',idx);
        string key = str.substr(0,idx);
        string value = str.substr(idx+1,endidx-idx-1);

        // cout << key << ":" << value << endl;

        if(key == "ip")
        {
            _ip = value;
        }else if(key == "port")
        {
            _port = atoi(value.c_str());
        }else if(key == "dbname")
        {
            _dbname = value;
        }
        else if(key == "username")
        {
            _username = value;
        }else if(key == "password")
        {
            _password = value;
        }else if(key == "initSize")
        {
            _initSize = atoi(value.c_str());
        }else if(key == "maxSize")
        {
            _maxSize = atoi(value.c_str());
        }else if(key == "maxIdleTime")
        {
            _maxIdleTime = atoi(value.c_str());
        }else if(key == "connectionTimeout")
        {
            _connectionTimeout = atoi(value.c_str());
        }

    }
    return true;
}
//连接池的构造函数
ConnectionPool::ConnectionPool()
{
    //加载配置项
    if(!loadConfigFile())
    {
        return;
    }
    /*
    创建初始数量的连接
    */
   for(int i =0;i<_initSize;++i)
   {
        Connection *p = new Connection();
        p->connect(_ip,_port,_username,_password,_dbname);
        _connectionQue.push(p);
        _connectionCnt++;
   }
   //启动一个新线程作为连接的生成者 linux thread  ==> pthread create
   thread produce(std::bind(&ConnectionPool::produceConnectionTask,this));
}
//专门负责生产新连接的线程函数
void ConnectionPool::produceConnectionTask()
{
    for(;;)
    {
        unique_lock<mutex> lock(_queueMutex);
        while(!_connectionQue.empty())
        {
            cv.wait(lock);   //队列不空，此时生产线程进入等待状态
        }
        //连接数量没有达到上限，继续创建新的连接
        if(_connectionCnt < _maxSize)
        {
            Connection *p = new Connection();
            p->connect(_ip,_port,_username,_password,_dbname);
            _connectionQue.push(p);
            _connectionCnt++;
        }
        //通知消费者线程，可以来消费连接了
        cv.notify_all();
    }
}