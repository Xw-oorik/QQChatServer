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



