# QQChatServer
基于muduo网络库实现的qq集群聊天服务器，工作在Nginx的Tcp负载均衡利用Redis发布订阅模式的消息队列实现集群服务器消息的转发

目录结构：linux下利用tree命令打印
      
        ---auto.sh自动编译脚本
        ├── bin
        │   ├── ChatClient
        │   └── ChatServer
        ├── build
        ├── CMakeLists.txt
        ├── include
        │   ├── public.hpp
        │   └── server
        │       ├── chatserver.hpp
        │       ├── chatservice.hpp
        │       ├── db
        │       │   └── db.hpp
        │       ├── model
        │       │   ├── friendmodel.hpp
        │       │   ├── group.hpp
        │       │   ├── groupmodel.hpp
        │       │   ├── groupuser.hpp
        │       │   ├── offlinemessagemodel.hpp
        │       │   ├── user.hpp
        │       │   └── usermodel.hpp
        │       └── redis
        │           └── redis.hpp
        ├── README.md
        ├── src
        │   ├── client
        │   │   ├── client.cpp
        │   │   └── CMakeLists.txt
        │   ├── CMakeLists.txt
        │   └── server
        │       ├── chatserver.cpp
        │       ├── chatservice.cpp
        │       ├── CMakeLists.txt
        │       ├── db
        │       │   └── db.cpp
        │       ├── main.cpp
        │       ├── model
        │       │   ├── friendmodel.cpp
        │       │   ├── groupmodel.cpp
        │       │   ├── offlinemessagemodel.cpp
        │       │   └── usermodel.cpp
        │       └── redis
        │           └── redis.cpp
        └── thirdparty
            └── json.hpp


单体的一个架构设计：
![image](https://user-images.githubusercontent.com/117898635/220850436-2dc7f305-6a6c-4a07-8e80-ea7f013c7495.png)

具体的数据库设计
建表代码如下
CREATE TABLE User(
  id INT NOT NULL AUTO_INCREMENT PRIMARY KEY COMMENT '用户id',
  name VARCHAR(50) NOT NULL  UNIQUE COMMENT '用户名',
  password VARCHAR(50) NOT NULL COMMENT '用户密码',
  state ENUM('online','offline') DEFAULT('offline') COMMENT '当前登录状态'
);
CREATE TABLE Friend(
  userid INT NOT NULL  COMMENT '用户id',
  friendid INT NOT NULL  COMMENT '好友id',
  PRIMARY KEY (userid,friendid)
);
CREATE TABLE AllGroup(
  groupid INT NOT NULL AUTO_INCREMENT PRIMARY KEY COMMENT '群组id',
  groupname VARCHAR(50) NOT NULL  UNIQUE COMMENT '群组名称',
  groupdesc VARCHAR(200) NOT NULL DEFAULT('') COMMENT'群组功能描述:学习群，聊天群等等'
);
CREATE TABLE GroupUser(
  groupid INT NOT NULL COMMENT '群组id',
  userid INT NOT NULL COMMENT '群组成员id',
  grouprole ENUM('creator','normal') NOT NULL DEFAULT('normal') COMMENT'组内成员角色身份',
  PRIMARY KEY (groupid,userid)
);
CREATE TABLE OfflineMessage(
  userid INT NOT NULL COMMENT '用户id',
  message VARCHAR(500) NOT NULL COMMENT'离线信息(Json字符串)'
);

对数据库层进行了一个ORM框架的设计：
![image](https://user-images.githubusercontent.com/117898635/220850847-02f6c369-8f5d-44d2-ae27-896beb882c50.png)

所有model对象就是封装了用来操作对应表数据的的方法，接收User/Group对象（映射对应表的字段），调用封装的数据库方法，去对各个表进行操作

引入了负载均衡之后的一个架构：
![image](https://user-images.githubusercontent.com/117898635/224493887-4ad06b90-5323-498d-a5ad-872060e51868.png)

