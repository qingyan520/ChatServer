#include"Mysql.hpp"
#include"User.hpp"
#include<vector>
using std::vector;
class FriendModel
{
  public:
    //添加好友
    void insert(int userid,int friendid)
    {
      char sql[1024]={0};
      sprintf(sql,"insert into Friend values(%d,%d);",userid,friendid);
      Mysql mysql;
      if(mysql.Connect())
      {
        mysql.Update(sql);
      }
    }
    //返回好用列表
    vector<User>query(int userid)
    {
      char sql[1024]={0};
      
      sprintf(sql,"select User.id,User.name,User.state from User join Friend on User.id=Friend.friendid where Friend.userid=%d;",userid);

      vector<User>vec;
      Mysql mysql;
      if(mysql.Connect())
      {
        MYSQL_RES*ret=mysql.Query(sql);
        if(ret!=nullptr)
        {
          MYSQL_ROW row;
          while((row=mysql_fetch_row(ret))!=nullptr)
          {
            User user;
            user.SetId(atoi(row[0]));
            user.SetName(row[1]);
            user.SetState(row[2]);
            vec.push_back(user);
          }
        }
      }
      return vec;
    }



};
