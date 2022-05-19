//该类主要负责业务逻辑的处理，比如登录，注册，发送消息

#pragma once
#include<iostream>
#include<functional>
#include<unordered_map>
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<muduo/base/Logging.h>
#include<functional>
#include"json.hpp"
#include"Public.hpp"
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace nlohmann;

//C++11新特性，using可以用来重命名
using MsgHandler=function<void (const TcpConnectionPtr&conn,json&js,Timestamp time)>;


//ChatService设置为单例模式
class ChatService
{
  //得到单例对象
  static ChatService*GetInstance()
  {
    static ChatService single;
    return &single;
  }

  //登录
  void Login(const TcpConnectionPtr&conn,json&js,Timestamp time);

  //注册
  void Reg(const TcpConnectionPtr&conn,json&js,Timestamp time);

  //点对点聊天
  void OneChat(const TcpConnectionPtr&conn,json&js,Timestamp time);

  //得到每一个消息所对应的回调函数
  MsgHandler GetHandler(int msgid);
  private:
  
  ChatService();

  ChatService(const ChatService&)=delete;


  unordered_map<int,MsgHandler>_HandlerMap;//记录每一个消息所对应的回调函数
};


//构造函数:负责对应的msgid回调函数的注册
ChatService::ChatService()
{
  //1.注册登录回调
  _HandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::Login,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});

  //2.注册注册的回调
  _HandlerMap.insert({REG_MSG,std::bind(&ChatService::Login,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});

  //3.注册聊天回调
  _HandlerMap.insert({ONECHAT_MSG,std::bind(&ChatService::OneChat,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
}

//得到每一个消息对应的回调函数
MsgHandler ChatService::GetHandler(int msgid)
{
  auto it=_HandlerMap.find(msgid);
  if(it==_HandlerMap.end())
  {
    return [=](const TcpConnectionPtr&,json&,Timestamp )->void{
      LOG_WARN<<msgid<<": can not found!";
    };
  }
  return it->second;
}


