#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<functional>
#include<thread>

#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>

#include"json.hpp"
#include"User.hpp"
#include"Public.hpp"
#include"Group.hpp"


using namespace std;
using json=nlohmann::json;


//记录当前系统登录的用户信息
User g_currnetUser;
//记录当前登录信息的好友列表
vector<User>g_currentUserFriendList;
//记录当前登录用户的群组列表想你洗
vector<Group>g_currentUserGroupList;

//控制主菜单程序
bool isMainMenuRunning=false;

//接收线程：接受发送过来的信息
void readTaskHandler(int clientfd);

//获取系统时间
string getCurrentTime();

//主聊天页面程序
void mainMenu();

//显示当前登录成功用户的基本信息
void showCurrentUserDate();



void menu()
{
  cout<<"\t\t+========================+"<<endl;
  cout<<"\t\t|        1.登录          |"<<endl;
  cout<<"\t\t|        2.注册          |"<<endl;
  cout<<"\t\t|        0.退出          |"<<endl;
  cout<<"\t\t+========================+"<<endl;
}

int Socket(string ip,int port)
{
    int listenfd=socket(AF_INET,SOCK_STREAM,0);
    if(listenfd<0)
    {
      cerr<<"socket create error!";
      exit(0);
    }

    struct sockaddr_in peer;
    peer.sin_family=AF_INET;
    peer.sin_port=htons(port);
    peer.sin_addr.s_addr=inet_addr(ip.c_str());

    if(connect(listenfd,(sockaddr*)&peer,sizeof(peer))<0)
    {
      cerr<<"connet error!"<<endl;
      exit(1);
    }
    return listenfd;
}


//登录函数
void login(int clientfd)
{
  cout<<"==========欢迎来到登录界面=========="<<endl;
  
  //输入登录账号和密码
  unsigned long long int id=0;
  char pwd[100]={0};
  cout<<"请输入账号:"<<endl;
  cin>>id;
  cin.get();
  cout<<"请输入密码:"<<endl;
  cin.getline(pwd,100);
  
  //构建json字符串
  json js;
  js["msgid"]=LOGIN_MSG;
  js["id"]=id;
  js["password"]=pwd;

  string request=js.dump();
  send(clientfd,request.c_str(),request.size(),0);

  //阻塞等待登录响应信息
  char recvbuf[1024000]={0};
  recv(clientfd,recvbuf,1024000,0);

  json response=json::parse(recvbuf);

  int error_number=response["error"].get<int>();

  if(error_number==0)
  {
    //登录成功
    g_currnetUser.SetId(response["id"].get<int>());
    g_currnetUser.SetName(response["name"]);
    
    //添加朋友列表
    if(response.contains("friend"))
    {
      vector<string>vec=response["friend"];
      for(string &str:vec)
      {
        User user;
        json u=json::parse(str);
        user.SetId(u["id"].get<int>());
        user.SetName(u["name"]);
        user.SetState(u["state"]);
        g_currentUserFriendList.push_back(user);
      }
    }
    cout<<"===============登录成功============="<<endl;
    //查看是否有离线消息
  }
  else
  {
    if(error_number==1)
    {
      cout<<"==========="<<response["errmsg"]<<"============"<<endl;
    }
    else if(error_number==2)
    {
      cout<<"==========="<<response["errmsg"]<<"============"<<endl;
    }
    else
    {
      cout<<"=========="<<"未知错误响应"<<endl;
    }
  }
}


//注册函数
void regist(int clientfd)
{ 
    cout<<"==========欢迎来到注册界面=========="<<endl;
    char name[100]={0};
    char pwd[100]={0};
    cout<<"请输入昵称:";
    cin.getline(name,100);
    cout<<"请输入密码:";
    cin.getline(pwd,100);

    json js;
    js["msgid"]=REG_MSG;
    js["name"]=name;
    js["password"]=pwd;
    //序列化json字符串
    string request=js.dump();
    //发送注册消息
    send(clientfd,request.c_str(),request.size(),0);
    
    //阻塞等待接受注册信息
    char recvbuf[1024]={0};
    recv(clientfd,recvbuf,1024,0);
    
    //反序列化json字符串
    json response=json::parse(recvbuf);
    
    int errnoid=response["errno"].get<int>();
    //错误消息返回0，代表注册成功
    if(errnoid==0)
    {
      cout<<"================注册成功=================="<<endl;
      cout<<"请记住您的账号和密码:"<<endl;
      cout<<"账号:"<<response["id"]<<endl;
      cout<<"密码:"<<pwd<<endl;
      cout<<"==========请返回登录界面进行登录=========="<<endl;
    }
    else
    {
      cout<<"==============注册失败================="<<endl;
      cout<<"该昵称已经被使用，请重新输入昵称"<<endl;
    }


}

int main()
{
    static string ip="39.105.98.201";
    int port=8081;
    //首先进行套接字通信，等到对应的clientfd
    int clientfd=Socket(ip,port);
    
    while(1)
    {
      menu();
      cout<<"请输入对应选项:"<<endl;
      unsigned long long int choice=0;
      cin>>choice;
      cin.get();

      switch(choice)
      {
        case 1:
          login(clientfd);
          break;
        case 2:
          regist(clientfd);
          break;
        case 0:
          cout<<"退出系统，欢迎下次使用"<<endl;
          exit(-1);
        default:
          cerr<<"输入错误,"<<endl;
          break;
      }
    }
    return 0;
}
