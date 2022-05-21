#pragma once 
#include<string>
#include<mysql.h>
#include<muduo/base/Logging.h>
using std::string;
using namespace muduo;

static string  user="root";
static string pwd="Ssxpb524";
static string db="chat";

class Mysql
{
  public:
    Mysql()
    {
      _conn=mysql_init(nullptr);
    }

    ~Mysql()
    {
      if(_conn!=nullptr)
      {
        mysql_close(_conn);
      }
    }
    
    bool Connect()
    {
        MYSQL*p=mysql_real_connect(_conn,"127.0.0.1",user.c_str(),pwd.c_str(),db.c_str(),3306,nullptr,0);
        if(p!=nullptr)
        {
          return true;
        }
        LOG_WARN<<"数据库连接出错！";
        LOG_WARN<<mysql_error(_conn);
        return false;
    }

    //负责数据的插入更新功能
    bool Update(string sql)
    {
      mysql_query(_conn,"set names utf8");
      if(mysql_query(_conn,sql.c_str()))
      {
        LOG_WARN<<__FILE__<<":"<<__LINE__<<":"<<sql<<": Update fail!";
        LOG_WARN<<mysql_error(_conn);
        return false;
      }
      return true;
    }

    //主要负责数据的查询，返回到MYSQL_RES这个结构体变量当中
    MYSQL_RES*Query(string sql)
    {
      mysql_query(_conn,"set names utf8");
      if(mysql_query(_conn,sql.c_str()))
      {
        LOG_WARN<<__FILE__<<":"<<__LINE__<<":"<<sql<<": Query fail";
        LOG_WARN<<mysql_error(_conn);
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
