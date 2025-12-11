#include "iostream"
#include "collection_pool.hpp"
#include "collection.hpp"
using namespace std;
int main()
{
    // std::cout << "test" <<std::endl;
    Connection conn;
    char sql[1024] = {0};
    sprintf(sql,"insert into users(name,age,sex) values('%s',%d,'%s') ",
    "zhangsan", 20,"F");
    conn.connect("127.0.0.1",3306,"test","123456","chat");
    conn.update(sql);
    return 0;
}