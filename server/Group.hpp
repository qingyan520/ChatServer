#include"GroupUser.hpp"
#include<vector>
using std::vector;

class Group
{
  public:
    
    Group(int _id=-1,string _name="",string _desc=""):
      id(_id),name(_name),desc(_desc)
      {

      }


    void SetId(int id)
    {
      this->id=id;
    }

    void SetName(string name)
    {
      this->name=name;
    }

    void SetDesc(string desc)
    {
      this->desc=desc;
    }

    //get方法
    int GetId()
    {
      return this->id;
    }

    string GetName()
    {
      return this->name;
    }

    string GetDesc()
    {
      return this->desc;
    }

    vector<GroupUser>&getUser()
    {
      return this->user;
    }

  private:
    int id;       //记录群组id
    string name;  //记录群组名称
    string desc;  //记录群组描述
    vector<GroupUser>user;    //记录群组成员
};
