#ifndef CHATSERVICE_HPP
#define CHATSERVICE_HPP

#include<iostream>
#include<muduo/net/TcpConnection.h>
#include<unordered_map>
#include<functional>
#include<mutex>
using namespace std;
using namespace muduo;
using namespace muduo::net;
#include"usermodel.hpp"
#include"offlinemessagemodel.hpp"
#include"friendmodel.hpp"
#include"groupmodel.hpp"
#include"redis.hpp"
#include"json.hpp"
using json=nlohmann::json;

// 消息id对应的回调，处理消息事件
using MsgHandler = function<void(const TcpConnectionPtr &con, json &js, Timestamp time)>;

//聊天服务器业务类
class ChatService
{
public:
    //获取单例对象的接口函数
    static ChatService* instance();
    //处理登录业务
    void login(const TcpConnectionPtr& con,json &js,Timestamp time);
    //处理注册业务
    void reg(const TcpConnectionPtr& con,json &js,Timestamp time);
    //获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    //一对一聊天服务
    void oneChat(const TcpConnectionPtr& con,json &js,Timestamp time);
    //添加好友业务
    void addFriend(const TcpConnectionPtr& con,json &js,Timestamp time);

    //创建群聊业务
    bool CreateGroup(const TcpConnectionPtr& con,json &js,Timestamp time);
    //加入群聊业务
    bool AddGroup(const TcpConnectionPtr& con,json &js,Timestamp time);
    //群组聊天业务
    void GroupChat(const TcpConnectionPtr& con,json &js,Timestamp time);
    
    //处理注销业务
    void loginout(const TcpConnectionPtr& con,json &js,Timestamp time);
    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr& con);
    //服务器异常，业务重置
    void reset();
    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int, string);
private:
    ChatService();

    //存储消息id和其对应的业务处理方法
    unordered_map<int,MsgHandler> _msgHandlerMap;

    //存储在线用户的通信连接  注意线程安全
    unordered_map<int,TcpConnectionPtr> _userConnMap;
     
    //定义互斥锁，保证_userConnMap线程安全
    mutex _connMutex;

    //所有model对象就是封装了用来操作对应表数据的的方法，接收User/Group对象（映射对应表的字段），调用封装的数据库方法，去对各个表进行操作

    //表数据操作类对象  
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    //redis操作对象
    Redis _redis;
};

#endif