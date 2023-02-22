#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>
using namespace muduo;
using namespace std;

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService chatservice;
    return &chatservice;
}
// 注册消息以及对应的Handler回调操作
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, bind(&ChatService::CreateGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, bind(&ChatService::AddGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, bind(&ChatService::GroupChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, bind(&ChatService::loginout, this, _1, _2, _3)});
    // 连接redis服务器
    if (_redis.connect())
    {
        // 设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}
// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志，msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认处理器，什么也不做
        return [=](const TcpConnectionPtr &con, json &js, Timestamp)
        {
            LOG_ERROR << "msgid:" << msgid << "cant find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}
// 服务器异常，业务重置
void ChatService::reset()
{
    // 把online用户设置成offline
    _userModel.resetState();
}
// 添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &con, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    // 添加好友信息
    _friendModel.insert(userid, friendid);
}
// 一对一聊天服务
void ChatService::oneChat(const TcpConnectionPtr &con, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> locker(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // toid 在线，转发消息  服务器主动推送给toid用户
            it->second->send(js.dump());
            return;
        }
    }
    // 在当前服务器上没有，看看在线不，在线就说明在别的服务器上，给消息队列里发消息
    User user = _userModel.query(toid);
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }
    // toid 不在线  存储离线消息
    _offlineMsgModel.insert(toid, js.dump());
}

// 创建群聊业务
bool ChatService::CreateGroup(const TcpConnectionPtr &con, json &js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    // 存储新创建的群聊信息
    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        // 存储群创建人信息
        if (_groupModel.addGroup(group.getId(), userid, "creator"))
        {
            return true;
        }
    }
    return false;
}
// 加入群聊业务
bool ChatService::AddGroup(const TcpConnectionPtr &con, json &js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    int groupid = js["groupid"].get<int>();
    if (_groupModel.addGroup(groupid, userid, "normal"))
    {
        return true;
    }
    return false;
}

// 群组聊天业务
void ChatService::GroupChat(const TcpConnectionPtr &con, json &js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    lock_guard<mutex> locker(_connMutex);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            // 转发消息
            it->second->send(js.dump());
        }
        else
        {
            // 查询toid是否在线
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump());
            }
            else
            {
                // 存储离线消息
                _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}
// 处理注销业务
void ChatService::loginout(const TcpConnectionPtr &con, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    {
        lock_guard<mutex> locker(_connMutex);
        // 在通讯map把用户删除
        auto it = _userConnMap.begin();
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }
    // 用户注销，相当于下线，在redis里取消订阅
    _redis.unsubscribe(id);
    // 更新用户状态信息
    User user(id, "", "", "offline");
    _userModel.updateState(user);
}
// 处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &con)
{
    User user;
    {
        lock_guard<mutex> locker(_connMutex);
        // 在通讯map把用户删除
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == con)
            {
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    // 用户注销，相当于下线，在redis里取消订阅
    _redis.unsubscribe(user.getId());
    // 更新用户登录状态
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}
// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    _offlineMsgModel.insert(userid, msg);
}
// 处理登录业务   id   password
void ChatService::login(const TcpConnectionPtr &con, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string pwd = js["password"];
    User user = _userModel.query(id);
    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            // 该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该账号已经登录,请重新输入";

            con->send(response.dump());
        }
        else
        {
            // 登录成功 记录用户连接信息  考虑线程安全
            {
                lock_guard<mutex> locker(_connMutex);
                _userConnMap.insert({id, con});
            }
            // id用户登录成功后，向redis订阅channel(id)
            _redis.subscribe(id);
            // 更新用户状态信息
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 查询该用户是否有离线消息
            vector<string> vv = _offlineMsgModel.query(id);
            if (!vv.empty())
            {
                response["offlinemsg"] = vv;
                // 读取完用户的所有离线消息之后，删除
                _offlineMsgModel.remove(id);
            }
            // 查询该用户的好友信息并返回
            vector<User> uservv = _friendModel.query(id);
            if (!uservv.empty())
            {
                vector<string> vec2;
                for (User &us : uservv)
                {
                    json js;
                    js["id"] = us.getId();
                    js["name"] = us.getName();
                    js["state"] = us.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }
            // 查询用户群组信息
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["groupid"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();

                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["userid"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;

                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }

            con->send(response.dump());
        }
    }
    else
    {
        // 用户不存在 ，或者存在 不过密码错误，登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或者密码错误";

        con->send(response.dump());
    }
}
// 处理注册业务  name  password
void ChatService::reg(const TcpConnectionPtr &con, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];
    User user; // 用户  开始注册
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user); // 看是否注册成功
    if (state)
    {
        // 成功   回给客户端的信息
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        con->send(response.dump());
    }
    else
    {
        // 失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        con->send(response.dump());
    }
}
