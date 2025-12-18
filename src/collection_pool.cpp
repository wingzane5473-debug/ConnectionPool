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
        p->refreshAliveTime(); //刷新一下开始空闲的起始时间
        _connectionQue.push(p);
        _connectionCnt++;
   }
   //启动一个新线程作为连接的生成者 linux thread  ==> pthread create
   thread produce(std::bind(&ConnectionPool::produceConnectionTask,this));
   produce.detach(); //分离线程

   //启动一个新的定时线程，扫描超过最大空闲时间的连接，进行回收
   thread scanner(std::bind(&ConnectionPool::scannerConnectionTask,this));
   scanner.detach();//分离线程
}
//专门负责生产新连接的线程函数
void ConnectionPool::produceConnectionTask()
{
    for(;;)
    {
        //lock的作用： 保证连接队列的线程安全，此时生产者线程独占连接队列
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
            p->refreshAliveTime(); //刷新一下开始空闲的起始时间
            _connectionQue.push(p);
            _connectionCnt++;
        }
        //通知消费者线程，可以来消费连接了
        cv.notify_all();
    }
}
//给外部提供接口，从连接池中获取一个可用的空闲连接。
shared_ptr<Connection> ConnectionPool::getConnection(){
    //lock的作用： 保证连接队列的线程安全
    //_queueMutex是ConnectionPool的成员变量，提供给生产者和消费者使用，此时消费者线程独占连接队列
    unique_lock<mutex> lock(_queueMutex);
    // 使用while循环，防止虚假唤醒和竞态条件
    while(_connectionQue.empty())  //连接队列空了
    {
        //为什么不能使用sleep？ 因为sleep会阻塞当前线程，无法被唤醒
        //使用条件变量等待，当有新连接时，生产者线程会进行通知
        // 等待并检查是否超时
        if(cv.wait_for(lock,chrono::milliseconds(_connectionTimeout))==cv_status::timeout)
        {
            // 超时后再次检查，防止在超时前一刻有连接
            if (_connectionQue.empty()) {
            LOG("获取空闲连接超时了...获取连接失败!");
            return nullptr;
        }
            // 如果有连接，退出循环
            break;
        }
        // 被唤醒，继续循环检查
    }
    //此时连接队列中有连接可用，为什么用shared_ptr？
    //shared_ptr析构时会把connection给delete掉，避免内存泄漏
    //此时相当于把connection给close掉了，因此需要自定义shared_ptr的删除器deleter，把连接归还给连接池
    shared_ptr<Connection> sp(_connectionQue.front(),
    [&](Connection *pcon)
    {
        unique_lock<mutex> lock(_queueMutex);
        pcon->refreshAliveTime(); //刷新一下开始空闲的起始时间
        _connectionQue.push(pcon);

    });
    _connectionQue.pop();

    if(_connectionQue.empty())
    {
        cv.notify_all(); //谁消费了队列中的最后一个连接，通知生产者线程生产新连接
    }

    
    return sp;
}

void ConnectionPool::scannerConnectionTask()
{
    for(;;)
    {
        //通过sleep模拟定时效果
        this_thread::sleep_for(chrono::seconds(_maxIdleTime)); //定时扫描
        //lock的作用： 保证连接队列的线程安全，此时扫描线程
        unique_lock<mutex> lock(_queueMutex);
        while(_connectionCnt > _initSize) //连接数量超过初始连接数量，
        {
            Connection *p = _connectionQue.front();
            //判断连接的空闲时间是否超过了最大空闲时间
            if(p->getAliveTime() >= (_maxIdleTime*1000)) //clock返回值单位是ms
            {
                //删除该连接
                _connectionQue.pop();
                _connectionCnt--;
                delete p;
            }
            else
            {
                break; //队头的连接没有超过最大空闲时间，后面的连接更不会超过，直接跳出循环
            }
        }
    }
}