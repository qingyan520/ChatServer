//该类主要负责业务逻辑的处理，比如登录，注册，发送消息

#pragma once
#include<iostream>
#include<functional>
#include<mutex>
#include<unordered_map>
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<muduo/base/Logging.h>
#include"UserModel.hpp"
#include"OfflineMsgModel.hpp"
#include"FriendModel.hpp"
#include"GroupModel.hpp"
#include"json.hpp"
#include"Public.hpp"
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace nlohmann;

//C++11新特性，using可以用来重命名
using MsgHandler=function<void (const TcpConnectionPtr&conn,json*js,Timestamp time)>;


//ChatService设置为单例模式
class ChatService
{
  public:
  //得到单例对象
  static ChatService*GetInstance()
  {
    static ChatService single;
    return &single;
  }

  //1.登录
  void Login(const TcpConnectionPtr&conn,json*js,Timestamp time);

  //2.注册
  void Reg(const TcpConnectionPtr&conn,json*js,Timestamp time);

  //3.点对点聊天
  void oneChat(const TcpConnectionPtr&conn,json*js,Timestamp time);

  //4.添加好友业务
  void addFriend(const TcpConnectionPtr&conn,json*js,Timestamp tiem);

  //5.创建群业务
  void createGroup(const TcpConnectionPtr&conn,json*js,Timestamp time);
    
  //6.加入群聊业务
  void addGroup(const TcpConnectionPtr&conn,json*js,Timestamp time);

  //7.群聊业务
  void chatGroup(const TcpConnectionPtr&conn,json*js,Timestamp time);

  //处理客户端异常退出
  void clientCloseException(const TcpConnectionPtr&conn);

  //处理服务器异常退出情况
  void reset();

  //得到每一个消息所对应的回调函数
  MsgHandler GetHandler(int msgid);
  private:
  
  ChatService();
  ChatService(const ChatService&)=delete;

  unordered_map<int,MsgHandler>_HandlerMap;//记录每一个消息所对应的回调函数

  UserModel _userModel;//负责数据库的增删
  OffMsgModel _offMsgModel;//负责离线消息的加入与删除
  FriendModel _friendModel;//负责添加好友的类
  GroupModel _groupModel;//负责群创建、加入群聊等功能

  unordered_map<int,TcpConnectionPtr>_userConnMap;//存储用户的连接信息

  mutex _connMutex;//互斥锁，保证_userConnMap的线程安全

};


//构造函数:负责对应的msgid回调函数的注册
ChatService::ChatService()
{
  //1.注册登录回调
  _HandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::Login,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});

  //2.注册注册的回调
  _HandlerMap.insert({REG_MSG,std::bind(&ChatService::Reg,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});

  //3.注册聊天回调
  _HandlerMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::oneChat,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});

  //4.添加好友回调
  _HandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});

  //5.创建群回调
  _HandlerMap.insert({CREATE_GROUP_MSG,std::bind(&ChatService::createGroup,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});

  //6.加入群聊回调
  _HandlerMap.insert({ADD_GROUP_MSG,std::bind(&ChatService::addGroup,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});

  //7.群聊回调
  _HandlerMap.insert({GROUP_CHAT_MSG,std::bind(&ChatService::chatGroup,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
}

//得到每一个消息对应的回调函数
MsgHandler ChatService::GetHandler(int msgid)
{
  auto it=_HandlerMap.find(msgid);
  if(it==_HandlerMap.end())
  {
    return [=](const TcpConnectionPtr&,json*,Timestamp )->void{
      LOG_WARN<<msgid<<": can not found!";
    };
  }
  return it->second;
}


//登录
void ChatService::Login(const TcpConnectionPtr&conn,json*js,Timestamp time)
{
  int id=(*js)["id"].get<int>();
  string pwd=(*js)["password"];

  
  User user=_userModel.query(id);
  LOG_INFO<<"数据库解析------"<<"id:"<<user.GetId()<<"name:"<<user.GetName()<<"pwd:"<<user.GetPwd();
  if(id!=-1&&id==user.GetId()&&pwd==user.GetPwd())
  {
    //该用户已经在别的地方登录了
      if(user.Getstate()=="online")
      {
        json response;
        response["msgid"]=LOGIN_MSG_ACK;
        response["error"]=2;
        response["errmsg"]="该账号已经登录，请重新输入账号!";
        conn->send(response.dump());
      }
      else
      {
        //登录成功，记录用户的连接信息
        {
          lock_guard<mutex>lock(_connMutex);
          _userConnMap.insert({id,conn});
        }

        user.SetState("online");

        _userModel.updateState(user);
        json response;
        response["msgid"]=LOGIN_MSG_ACK;
        response["errno"]=0;
        response["id"]=user.GetId();
        response["name"]=user.GetName();
        
        vector<string> msg=_offMsgModel.query(id);
        if(!msg.empty())
        {
          //添加消息之后将其从离线消息表中删除
          response["OfflineMsg"]=msg;
          _offMsgModel.remove(id);
        }


        //查询用户的好友信息并返回
        vector<User>userVec=_friendModel.query(id);
        if(!userVec.empty())
        {
          vector<string>vec2;
          for(User&user:userVec)
          {
            json js;
            js["id"]=user.GetId();
            js["name"]=user.GetName();
            js["state"]=user.Getstate();

            vec2.push_back(js.dump());
          }
          response["friend"]=vec2;
        }

        conn->send(response.dump());
      }
  }
  else
  {
    //该用户不存在，登录失败
    json response;
    response["msgid"]=LOGIN_MSG_ACK;
    response["error"]=1;
    response["errmsg"]="账号或密码错误！";
    conn->send(response.dump());
  }
}


//注册回调函数
void ChatService::Reg(const TcpConnectionPtr&conn,json*js,Timestamp time)
{
  string name=(*js)["name"];
  string pwd=(*js)["password"];
  User user;
  user.SetName(name);
  user.SetPwd(pwd);
  bool state=_userModel.insert(user);
  if(state)
  {
    json response;
    response["msgid"]=REG_MSG_ACK;
    response["errno"]=0;
    response["id"]=user.GetId();
    conn->send(response.dump());
  }
  else
  {
    //注册失败
    json response;
    response["msgid"]=REG_MSG_ACK;
    response["errno"]=1;
    conn->send(response.dump());
  }
}

//联系
void ChatService::oneChat(const TcpConnectionPtr&conn,json*js,Timestamp time)
{
  int toid=(*js)["to"].get<int>();
  {
    lock_guard<mutex>lock(_connMutex);
    auto it=_userConnMap.find(toid);
    if(it!=_userConnMap.end())
    {
      //toid在线，转发消息，服务器主动推送消息
      it->second->send((*js).dump());
      return;
    }
  }
  _offMsgModel.insert(toid,(*js).dump());
}


//处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr&conn)
{
  User user;
  {
    lock_guard<mutex>lock(_connMutex);
    for(auto it=_userConnMap.begin();it!=_userConnMap.end();++it)
    {
      if(it->second==conn)
      {
        //从map表中删除
        user.SetId(it->first);
        _userConnMap.erase(it);
        break;
      }
    }
  }
 //跟新用户信息
 if(user.GetId()!=-1)
 {
   user.SetState("offline");
   _userModel.updateState(user);
 }
  
}


//添加好友业务
void ChatService::addFriend(const TcpConnectionPtr&conn,json*js,Timestamp time)
{
  int userid=(*js)["id"].get<int>();
  int friendid=(*js)["friendid"].get<int>();
  _friendModel.insert(userid,friendid);
}



//服务器异常退出问题
void ChatService::reset()
{
    _userModel.updateAllState();
}

//创建群业务
void ChatService::createGroup(const TcpConnectionPtr&conn,json*js,Timestamp time)
{
  int userid=(*js)["id"].get<int>();
  string groupname=(*js)["groupname"];
  string groupdesc=(*js)["groupdesc"];
  Group group(-1,groupname,groupdesc);
  if(_groupModel.createGroup(group))
  {
    _groupModel.addGroup(userid,group.GetId(),"creator");
  }
}



//加入群聊业务
void ChatService::addGroup(const TcpConnectionPtr&conn,json*js,Timestamp time)
{
  int userid=(*js)["id"].get<int>();
  int groupid=(*js)["groupid"].get<int>();
  _groupModel.addGroup(userid,groupid,"normal");
}



//群聊业务
void ChatService::chatGroup(const TcpConnectionPtr&conn,json*js,Timestamp time)
{
  int userid=(*js)["id"].get<int>();
  int groupid=(*js)["groupid"].get<int>();
  
  //得到在同一个组里面的其他人的id
  vector<int>vecid=_groupModel.queryGroupUsers(userid,groupid);
  lock_guard<mutex>lock(_connMutex);

  for(auto e:vecid)
  {
    auto it=_userConnMap.find(e);
    if(it!=_userConnMap.end())
    {
      //发送消息
      it->second->send((*js).dump());
    }
    else
    {
      //目标不在线，存储离线消息，等待上线后发送
      _offMsgModel.insert(e,(*js).dump());
    }
  }
}

