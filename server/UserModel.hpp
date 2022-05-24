#include"User.hpp"
#include"Mysql.hpp"

class UserModel
{
  public:
    //该函数主要用于注册，向数据库中添加数据，然后判断添加是否成功
    bool insert(User&user)
    {
      //1.拼接sql语句
      char sql[1024]={0};
      sprintf(sql,"insert into User(name,password,state) values('%s','%s','%s');",user.GetName().c_str(),user.GetPwd().c_str(),user.Getstate().c_str());
      Mysql mysql;
      if(mysql.Connect())
      {
        if(mysql.Update(sql))
        {
          //添加成功，设置id
          user.SetId(mysql_insert_id(mysql.GetConnect()));
           return true;
        }
      }
      return false;
    }

    //该函数主要用于登录，验证账号和密码是否错误
    User query(int id)
    {
        char sql[1024]={0};
        sprintf(sql,"select * from User where id=%d;",id);
        Mysql mysql;
        User user;
        if(mysql.Connect())
        {
           MYSQL_RES*ret=mysql.Query(sql);
           if(ret!=nullptr)
           {
             MYSQL_ROW row=mysql_fetch_row(ret);
              
             if(row!=nullptr)
             {
                user.SetId(atoi(row[0]));
                user.SetName(row[1]);
                user.SetPwd(row[2]);
                user.SetState(row[3]);
             }
             mysql_free_result(ret);
           }
        }
        return user;
    }

    bool updateState(User user)
    {
      char sql[1024]={0};
      sprintf(sql,"update User set state='%s' where id=%d;",user.Getstate().c_str(),user.GetId());

      Mysql mysql;
      if(mysql.Connect())
      {
        if(mysql.Update(sql))
        {
          return true;
        }
      }
      return false;
    }
    
    
    bool updateAllState()
    {
      //退出前将所有人状态设置为offline
        char sql[1024]="update User set state='offline'";
        
        Mysql mysql;
        if(mysql.Connect())
        {
          if(mysql.Update(sql))
          {
            return true;
          }
        }
        return false;
    }


};
