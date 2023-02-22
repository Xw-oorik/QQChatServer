#ifndef USER_HPP
#define USER_HPP

#include<string>
using namespace std;

// 匹配user表的ORM类  映射表的字段的
class User
{
public:
    User(int _id = -1, string _name = "", string pwd = "", string _state = "offline") : id(_id),
                                                                                        name(_name),
                                                                                        password(pwd),
                                                                                        state(_state) {}

    void setId(int _id) { this->id = _id; }
    void setName(string _name) { this->name = _name; }
    void setPwd(string pwd) { this->password = pwd; }
    void setState(string _state) { this->state = _state; }

    int getId() { return this->id; }
    string getName() { return this->name; }
    string getPwd() { return this->password; }
    string getState() { return this->state; }

private:
    int id;
    string name;
    string password;
    string state;
};

#endif