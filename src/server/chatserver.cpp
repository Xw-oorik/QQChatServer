#include"chatserver.hpp"
#include"json.hpp"
#include"chatservice.hpp"

#include<iostream>
#include<functional>
#include<string>
using namespace std;
using namespace placeholders;
using json=nlohmann::json;

//初始化聊天服务器对象
ChatServer:: ChatServer(EventLoop* loop,//事件循环
            const InetAddress& listenAddr,//ip port
            const string& nameArg)//服务器的名字
            :_server(loop,listenAddr,nameArg),_loop(loop)
{
     //给服务器注册用户链接的回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));
    
    //注册消息的回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));

    //设置线程数量
    _server.setThreadNum(4);

}

//启动聊天服务器
void ChatServer::start()
{
    _server.start();
}

//上报链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr& con)
{
    //客户端断开链接
    if(!con->connected()){
        //处理客户端异常关闭，要修改状态不然下次登录还是online状态
        ChatService::instance()->clientCloseException(con);
        con->shutdown();
    }
}

//上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &con,Buffer *buffer,Timestamp time)
{
    string buf=buffer->retrieveAllAsString();
    //数据的反序列化
    json js=json::parse(buf);
    //使网络模块和业务模块代码完全解耦 ，分开
    //通过js[msgid]拿到对应业务的handler回调，把con js time传进去

    auto msgHandler=ChatService::instance()->getHandler(js["msgid"].get<int>());
    //回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(con,js,time);
}