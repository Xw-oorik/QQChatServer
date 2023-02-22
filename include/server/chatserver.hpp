#ifndef CHATSERVER_HPP
#define CHATSERVER_HPP

#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

//聊天服务器的主类
class ChatServer
{
public:
    //初始化聊天服务器对象
    ChatServer(EventLoop* loop,//事件循环
            const InetAddress& listenAddr,//ip port
            const string& nameArg);//服务器的名字
    //开启事件循环
    void start();
private:
    //专门用来处理用户连接和断开的  epoll listenfd accept
    void onConnection(const TcpConnectionPtr& con);

    // 当有新数据时调用,专门处理用户读写事件的
    void onMessage(const TcpConnectionPtr &con,//连接
                    Buffer *buffer,//缓冲区
                    muduo::Timestamp time);//接收到数据的时间信息

    TcpServer _server;//组合的muduo库，实现服务器功能的类对象
    EventLoop *_loop;//指向事件循环对象的指针
};

#endif