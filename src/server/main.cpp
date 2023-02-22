#include"chatserver.hpp"
#include"chatservice.hpp"
#include<iostream>
#include<signal.h>
using namespace std;

//处理服务器Ctrl+c异常退出后，用户的状态变化
void restHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}
int main(int argc,char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << endl;
        exit(-1);
    }
    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT,restHandler);

    EventLoop loop;//特别像创建了一个epoll
    InetAddress addr("ip",port);
    ChatServer server(&loop,addr,"ChatServer");
    server.start();//启动服务  listenfd 添加epoll_ctl
    loop.loop();//以阻塞的方式调用等待新用户连接/已连接用户的读写操作  类似epoll_wait
    return 0;
}