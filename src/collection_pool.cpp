#include "iostream"
#include "collection_pool.hpp"

//线程安全的懒汉单例模式
ConnectionPool* ConnectionPool::getConnectionPool()
{
    static ConnectionPool pool; //静态对象只初始化一次。
    return &pool;
}
