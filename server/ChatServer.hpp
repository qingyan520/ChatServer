#include<iostream>
#include<functional>
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<muduo/base/Logging.h>
#include"json.hpp"
#include"ChatService.hpp"
using namespace std;
using namespace muduo;
using namespace muduo::net;
using json=nlohmann::json;
class ChatServer
{
  public:
    //初始化服务器
    ChatServer(EventLoop*loop,InetAddress&addr,string name);
    
    //启动服务器
    void start();
    
  private:
    //设置连接时回调
    void onConnection(const TcpConnectionPtr&conn);

    //设置消息处理回调
    void onMessage(const TcpConnectionPtr&conn,Buffer*buffer,Timestamp time);
    TcpServer _server;
    EventLoop*_loop;
};

//构造函数
ChatServer::ChatServer( EventLoop*loop,InetAddress&addr,string name):
  _server(loop,addr,name),
  _loop(loop)
{
  //1.绑定连接回调
  _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,std::placeholders::_1));
  
  //2.设置消息处理回调
  _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

  //设置线程数量
  _server.setThreadNum(4);
}

//启动服务器
void ChatServer::start()
{
  _server.start();
}

//连接时的回调
void ChatServer::onConnection(const TcpConnectionPtr&conn)
{
  if(!conn->connected())
  {
    ChatService::GetInstance()->clientCloseException(conn);
  }
}


//消息处理的回调
void ChatServer::onMessage(const TcpConnectionPtr&conn,Buffer*buffer,Timestamp time)
{
  string buf=buffer->retrieveAllAsString();
  json js=json::parse(buf);
  cout<<js["msgid"].get<int>()<<endl;
  int msgid=js["msgid"].get<int>();
  auto Handler=ChatService::GetInstance()->GetHandler(msgid);
  Handler(conn,&js,time);
}
