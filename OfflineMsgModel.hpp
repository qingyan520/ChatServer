#include"Mysql.hpp"
#include<vector>
using std::vector;
class OffMsgModel
{
  public:
    void insert(int toid,string msg)
    {
      char sql[1024]={0};
      sprintf(sql,"insert into OfflineMessage(userid,message) values(%d,'%s');",toid,msg.c_str());
      Mysql mysql;
      if(mysql.Connect())
      {
         mysql.Update(sql);
      }
    }


    //等到用户上线之后读取完离线消息之后删除离线消息
    void remove(int id)
    {
      char sql[1024]={0};
      sprintf(sql,"delete from OfflineMessafe where userid=%d;",id);
      Mysql mysql;
      if(mysql.Connect())
      {
        mysql.Update(sql);
      }
    }


    //获得离线消息
    vector<string> query(int userid)
    {
      char sql[1024]={0};
      sprintf(sql,"select message from OfflineMessge where id='%d';",userid);
      vector<string>vec;
      Mysql mysql;
      if(mysql.Connect())
      {
        MYSQL_RES*ret=mysql.Query(sql);
        if(ret!=nullptr)
        {
          MYSQL_ROW row;
          while((row=mysql_fetch_row(ret))!=nullptr)
          {
            vec.push_back(row[0]);
          }
          mysql_free_result(ret);
        }

      }
      return vec;
    }

};
