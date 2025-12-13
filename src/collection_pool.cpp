#include <string>
#include "collection_pool.hpp"
#include "public.h"
//线程安全的懒汉单例模式
ConnectionPool* ConnectionPool::getConnectionPool()
{
    static ConnectionPool pool; //静态对象只初始化一次。自动进行lock与unlock
    return &pool;
}
ConnectionPool::ConnectionPool()
{

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

        cout << key << ":" << value << endl;
    }

}