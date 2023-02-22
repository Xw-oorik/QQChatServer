#include "db.hpp"
#include "groupmodel.hpp"
// 创建群组
bool GroupModel::createGroup(Group &group)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupname,groupdesc) values('%s', '%s');",
            group.getName().c_str(), group.getDesc().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;
}
// 加入群组
bool GroupModel::addGroup(int groupid, int userid, string grouprole)
{
    // 组装sql语句
    char sql[512] = {0};
    sprintf(sql, "insert into GroupUser values(%d,%d,'%s');",
            groupid, userid, grouprole.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}
// 查询用户所在群组信息
vector<Group> GroupModel::queryGroups(int userid)
{
    /*
    先根据userid在的groupuser表和AllGroup中查询出该用户所属的群组信息
    再根据群组信息，查询属于该群组的所有用户的userid，并且和user表进行多表联合查询，查询用户详细信息
    */
    char sql[512] = {0};
    sprintf(sql, "select a.groupid,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on a.groupid=b.groupid where b.userid=%d;",
            userid);

    vector<Group> groupVec;

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    // 查询群组的用户信息
    for (Group &group : groupVec)
    {
        sprintf(sql, "select a.id,a.name,a.state,b.grouprole from User a inner join GroupUser b on b.userid=a.id where b.groupid=%d;",
                group.getId());
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return groupVec;
}
// 根据指定groupid查询群组用户id列表，除userid自己，用在用户群聊业务给群组其他成员群发信息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[512] = {0};
    sprintf(sql, "select userid from GroupUser where groupid =%d and userid != %d;", groupid, userid);

    vector<int> idVec;

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}
