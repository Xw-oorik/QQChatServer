#ifndef USERMODEL_HPP
#define USERMODEL_HPP

#include "user.hpp"

// 具体User这张表的数据业务操作类

class UserModel
{
public:
    // User表的增加方法
    bool insert(User &user);
    //根据用户账号查询用户信息
    User query(int id);
    //更新用户的状态信息
    bool updateState(User user);

    //重置用户状态信息 ,用在服务器异常
    void resetState();
};

#endif
