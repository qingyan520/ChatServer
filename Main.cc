#include"ChatServer.hpp"

int main()
{
  EventLoop loop;
  InetAddress addr("172.16.47.167 ",8080);
  ChatServer server(&loop,addr,"ChatServer");
  server.start();
  loop.loop();
}
