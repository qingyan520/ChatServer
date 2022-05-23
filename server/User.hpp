#pragma once
#include<string>
using namespace std;
class User
{
  public:

    //默认构造函数
    User(int id=-1,string name="",string password="",string State="offline"):
      _id(id),_name(name),_password(password),_state(State)
      {

      }

    //设置对应的属性值
    void SetId(int id)
    {
      this->_id=id;
    }

    void SetName(string name)
    {
      this->_name=name;
    }
    
    void SetPwd(string pwd)
    {
      this->_password=pwd;
    }

    void SetState(string state)
    {
      this->_state=state;
    }

    //取得对应的值
    int GetId()
    {
      return this->_id;
    }

    string GetName()
    {
      return this->_name;
    }

    string GetPwd()
    {
      return this->_password;
    }

    string Getstate()
    {
      return this->_state;
    }

  protected:
    int _id;
    string _name;
    string _password;
    string _state;
};
