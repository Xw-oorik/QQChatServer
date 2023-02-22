#ifndef GROUPMODEL_HPP
#define GROUPMODEL_HPP

#include"group.hpp"
#include<vector>
#include<string>
using namespace std;

//维护群组信息的操作方法接口
class GroupModel
{
public:
    //创建群组
    bool createGroup(Group &group);
    //加入群组
    bool addGroup(int groupid,int userid,string grouprole);
    //查询用户所在群组信息
    vector<Group>queryGroups(int userid);
    //根据指定groupid查询群组用户id列表，除userid自己，用在用户群聊业务给群组其他成员群发信息
    vector<int>queryGroupUsers(int userid,int groupid);
private:
};

#endif