#include"ChatServer.hpp"
#include<signal.h>

void handler(int signo)
{
  LOG_INFO<<"Ctrl+c信号处理";
  ChatService::GetInstance()->reset();
}
int main()
{
  
  signal(SIGINT,handler);

  EventLoop loop;
  InetAddress addr("172.16.47.167 ",8080);
  ChatServer server(&loop,addr,"ChatServer");
  server.start();
  loop.loop();
}
