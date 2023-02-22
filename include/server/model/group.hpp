#ifndef GROUP_HPP
#define GROUP_HPP
#include "groupuser.hpp"
#include <string>
#include <vector>
using namespace std;

// 类似User,映射数据字段的
class Group
{
public:
    Group(int _id = -1, string _name = "", string desc = ""):groupid(_id),groupname(_name),groupdesc(desc)
    {
    }

    void setId(int _id) { this->groupid = _id; }
    void setName(string _name) { this->groupname = _name; }
    void setDesc(string desc) { this->groupdesc = desc; }

    int getId() { return this->groupid; }
    string getName() { return this->groupname; }
    string getDesc() { return this->groupdesc; }
    vector<GroupUser> &getUsers() { return this->users; }

private:
    int groupid;
    string groupname;
    string groupdesc;
    vector<GroupUser> users;
};

#endif