#include "iostream"
#include "collection_pool.hpp"
#include "collection.hpp"
#include <cstdio>
#include <cstring>
using namespace std;


int main()
{
    /*测试mysql的connection类是否正常*/
    // std::cout << "test" <<std::endl;
    // Connection conn;
    // char sql[1024] = {0};
    // sprintf(sql,"insert into users(name,age,sex) values('%s',%d,'%s') ",
    // "zhangsan", 20,"F");
    // conn.connect("127.0.0.1",3306,"test","123456","chat");
    // conn.update(sql);

    /*测试loadconfigfile功能是否正常*/
    // ConnectionPool *connpool = ConnectionPool::getConnectionPool();
    // connpool->loadConfigFile();
    return 0;
}