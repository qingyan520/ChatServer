#pragma once
#include"Group.hpp"
#include"Mysql.hpp"

class GroupModel
{
  public:
    
    //创建群组
    inline bool createGroup(Group&group);

    //加入群组
    inline void addGroup(int userid,int groupid,string role);

    //查询用户所在群组的信息
    inline vector<Group>queryGroups(int userid);

    //根据指定的groupid查询群组用户id列表，出userid自己，主要用户群聊业务给群组内其它成员发消息
    inline vector<int>queryGroupUsers(int userid,int groupid);
};

inline bool GroupModel::createGroup(Group&group)
{
  char sql[1024]={0};
  sprintf(sql,"insert into AllGroup(groupname,groupdesc) values('%s','%s');",group.GetName().c_str(),group.GetDesc().c_str());
  
  Mysql mysql;
  if(mysql.Connect())
  {
    if(mysql.Update(sql))
    {
      group.SetId(mysql_insert_id(mysql.GetConnect()));
      return true;
    }
  }
  return false;
}

inline void GroupModel::addGroup(int userid,int groupid,string role)
{
  //组装sql语句
  char sql[1024]={0};
  sprintf(sql,"insert into GroupUser(groupid,userid,grouprole) values(%d,%d,'%s');",groupid,userid,role.c_str());
  Mysql mysql;
  if(mysql.Connect())
  {
    mysql.Update(sql);
  }
}



//查询用户所在群组的信息
inline vector<Group> GroupModel:: queryGroups(int userid)
{
  char sql[1024]={0};
  //1.首先选取userid的所有群组
  sprintf(sql,"select AllGroup.id,AllGroup.groupname,AllGroup.groupdesc from AllGroup join GroupUser on AllGroup.id=GroupUser.groupid where GroupUser.userid=%d;",userid);
  vector<Group>groupvec;
  Mysql mysql;
  if(mysql.Connect())
  {
    MYSQL_RES*ret=mysql.Query(sql);
    if(ret!=nullptr)
    {
      MYSQL_ROW row;
      while((row=mysql_fetch_row(ret))!=nullptr)
      {
        Group group;
        group.SetId(atoi(row[0]));
        group.SetName(row[1]);
        group.SetDesc(row[2]);

        groupvec.push_back(group);
      }
      mysql_free_result(ret);
    }
    
  }


  //向每一个群组中添加对应人员
  
  for(Group&group:groupvec)
  {
    char sql2[1024]{0};
    sprintf(sql2,"select User.id,User.name,User.state,GroupUser.grouprole from User join GroupUser on User.id=GroupUser.id where GroupUser.groupid=%d;",group.GetId());

    MYSQL_RES*ret=mysql.Query(sql2);
    if(ret!=nullptr)
    {
      MYSQL_ROW row;
      while((row=mysql_fetch_row(ret))!=nullptr)
      {
          GroupUser user;
          user.SetId(atoi(row[0]));
          user.SetName(row[1]);
          user.SetState(row[2]);
          user.SetRole(row[3]);

          group.getUser().push_back(user);
      }
      mysql_free_result(ret);
    }
  }

  return groupvec;
}


//根据指定的groupid查询群组用户id列表，出userid自己，主要用户群聊业务给群组内其它成员发消息
inline vector<int>GroupModel::queryGroupUsers(int userid,int groupid)
{
  char sql[1024]={0};
  sprintf(sql,"select userid from GroupUser where groupid=%d and userid != %d;",groupid,userid);
  vector<int>vec;
  Mysql mysql;
  if(mysql.Connect())
  {
    MYSQL_RES*ret=mysql.Query(sql);
    if(ret!=nullptr)
    {
      MYSQL_ROW row;
      while((row=mysql_fetch_row(ret))!=nullptr)
      {
        vec.push_back(atoi(row[0]));
      }

      mysql_free_result(ret);
    }
  }
  return vec;
}

