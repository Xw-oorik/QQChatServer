#ifndef GROUPUSER_HPP
#define GROUPUSER_HPP

#include "user.hpp"
// 群组用户，多了一个role角色信息，从User类直接继承，复用他的其他信息
class GroupUser : public User
{
public:
    void setRole(string role)
    {
        this->grouprole = role;
    }
    string getRole()
    {
        return grouprole;
    }

private:
    string grouprole;
};

#endif