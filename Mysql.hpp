#pragma once 
#include<string>
#include<mysql.h>
#include<muduo/base/Logging.h>
using std::string;
using namespace muduo;

static string  user="chat";
static string pwd="123456";
static string db="chat";

class Mysql
{
  public:
    Mysql()
    {
      _conn=mysql_init(nullptr);
    }
    
    bool Connect()
    {
        MYSQL*p=mysql_real_connect(_conn,"127.0.0.1",user.c_str(),pwd.c_str(),db.c_str(),3306,nullptr,0);
        if(p!=nullptr)
        {
          return true;
        }
        return false;
    }

    //负责数据的插入更新功能
    bool Update(string sql)
    {
      if(mysql_query(_conn,sql.c_str()))
      {
        LOG_WARN<<__FILE__<<":"<<__LINE__<<":"<<sql<<": Update fail!";
        return false;
      }
      return true;
    }

    //主要负责数据的查询，返回到MYSQL_RES这个结构体变量当中
    MYSQL_RES*Query(string sql)
    {
      if(mysql_query(_conn,sql.c_str()))
      {
        LOG_WARN<<__FILE__<<":"<<__LINE__<<":"<<sql<<": Query fail";
        return nullptr;
      }
      return mysql_use_result(_conn);
    }
    
    MYSQL*GetConnect()
    {
      return _conn;
    }
  private:
    MYSQL*_conn;
};
