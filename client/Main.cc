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
void mainMenu(int clientfd);

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
    
    //查看群组列表
    
    //查看是否有离线消息
    

    //显示自己的id,name,
    //显示群组信息
    //显示好友信息
    
    //线程分离，创建一个专门用来读的线程读取服务器发送回来的内容
    thread ReadThread(readTaskHandler,clientfd);
    ReadThread.detach();
    //进入聊天客户端的主页面
    mainMenu(clientfd);
    
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



//读线程，专门负责进行读取内容
void readTaskHandler(int clientfd)
{
  while(1)
  {
    char buf[1024000]={0};
    int len=recv(clientfd,buf,1024000,0);
    if(len<0)
    {
      exit(-1);
      cerr<<"子线程读取出错！！！"<<endl;
    }

    json js=json::parse(buf);
    int msgid=js["msgid"].get<int>();
    
    if(msgid==ONE_CHAT_MSG)
    {
      cout <<"[收到一个个人消息]"<< js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
            continue;
    }
    else if(msgid==GROUP_CHAT_MSG)
    {
      cout<<"[收到一个群消息]"<<"["<<js["time"]<<"][来自群:"<<js["groupname"]<<"]"<<"[群id:"<<js["groupid"]<<"]"<<"[发送者id:"<<js["id"]<<"]"<<"[name:"<<js["name"]<<"] msg:"<<js["msg"]<<endl;
      continue;
    }
    else
    {
      cout<<js<<endl;
    }
    

  }
}

//帮助界面
void help(int fd=0,string str="");
//一对一聊天
void chat(int ,string);
//添加好友
void addfriend(int,string);
//创建群组
void creategroup(int,string);
//群组聊天
void groupchat(int,string);
//添加群组
void addgroup(int,string);
//注销登录
void loginout(int,string)
{

}

//系统支持的客户端命令列表
unordered_map<string,string>commandMap={
  {"help","显示所有支持的命令，格式help"},
  {"chat","一对一聊天，格式chat:friendid:message"},
  {"addfriend","添加好友，格式addfriend:friendid"},
  {"creategroup","创建群组，格式creategroup:groupname,groupdesc"},
  {"addgroup","加入群组，格式addgroup:groupid"},
  {"groupchat","群聊，格式groupchat:groupid:message"},
  {"loginout","注销，格式loginout"}
};


// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}
};






void mainMenu(int clientfd)
{
   help();
   while(1)
   {
      cout<<"请输入对应命令:"<<endl;
      string commandbuf;
      cin>>commandbuf;
      cin.get();
      string command;
      //找到第一个：进行分割
      int idx=commandbuf.find(":");
      if(-1==idx)
      {
        command=commandbuf;
      }
      else
      {
        command=commandbuf.substr(0,idx);
      }
      
      auto it=commandHandlerMap.find(command);
      if(it==commandHandlerMap.end())
      {
        cerr<<"命令输入错误，请重新输入"<<endl;
        continue;
      }
      it->second(clientfd,commandbuf.substr(idx+1));
   }
}


void help(int,string)
{
  cout<<"命令列表>>>"<<endl;

  for(auto e:commandMap)
  {
    cout<<e.first<<":"<<e.second<<endl;
  }
  cout<<endl;
}




//一对一聊天
void chat(int clientfd,string str)
{
    //再次判断输入是否正确
    int idx=str.find(":");
    if(idx==-1)
    {
      cerr<<"======命令格式错误，请重新输入======"<<endl;
      return ;
    }
    
    int friendid=atoi(str.substr(0,idx).c_str());
    if(friendid==0)
    {
      cerr<<"======命令格式错误，请重新输入======"<<endl;
    }
    string msg=str.substr(idx+1);
    json js;
    js["msgid"]=ONE_CHAT_MSG;
    js["id"]=g_currnetUser.GetId();
    js["name"]=g_currnetUser.GetName();
    js["toid"]=friendid;
    js["msg"]=msg;
    js["time"]=getCurrentTime();

    string buffer=js.dump();
    cout<<buffer<<endl;
    int len=send(clientfd,buffer.c_str(),buffer.size(),0);
    if(-1==len)
    {
      cerr<<"send chat msg error";
    }
}


//添加好友
void addfriend(int clientfd,string str)
{
  
  int friendid=atoi(str.c_str());
  if(friendid==0)
  {
    cerr<<"======命令参数错误，请重新输入======"<<endl;
    return;
  }

  json js;
  js["msgid"]=ADD_FRIEND_MSG;
  js["id"]=g_currnetUser.GetId();
  js["friendid"]=friendid;

  string buffer=js.dump();

  int len=send(clientfd,buffer.c_str(),buffer.size(),0);
  if(len<0)
  {
    cerr<<"send add_friend_msg error"<<endl;
  }
}

//创建群组
void creategroup(int clientfd,string str)
{
  int idx=str.find(":");
  if(idx==-1)
  {
    cerr<<"======命令格式错误，请重新输入======"<<endl;
    return;
  }
  string groupname=str.substr(0,idx);
  string groupdesc=str.substr(idx+1);

  json js;
  js["id"]=g_currnetUser.GetId();
  js["groupdesc"]=groupdesc;
  js["groupname"]=groupname;
  js["msgid"]=CREATE_GROUP_MSG;

  string buffer=js.dump();
  int len=send(clientfd,buffer.c_str(),buffer.size(),0);
  if(len<0)
  {
    cerr<<"send create_group_msg error"<<endl;
  }
}


//加入群组业务
void addgroup(int clientfd,string str)
{
  int idx=str.find(":");
  if(idx==-1)
  {
    cerr<<"======命令格式错误，请重新输入======"<<endl;
    return;
  }
  int groupid=atoi(str.substr(0,idx).c_str());
  if(groupid==0)
  {
    cerr<<"======命令格式错误，请重新输入======"<<endl;
    return;
  }
  json js;
  js["msgid"]=ADD_GROUP_MSG;
  js["groupid"]=groupid;
  js["id"]=g_currnetUser.GetId();


  string buffer=js.dump();
  int len=send(clientfd,buffer.c_str(),buffer.size(),0);
  if(len<0)
  {
    cerr<<"send add_group_msg error"<<endl;
  }
}


//群组聊天功能
void groupchat(int clientfd,string str)
{
  int idx=str.find(":");
  if(idx==-1)
  {
    cerr<<"======命令输入错误，请重新输入======"<<endl;
    return;
  }

  int groupid=atoi(str.substr(0,idx).c_str());
  if(groupid==0)
  {
    cerr<<"======命令输入错误，请重新输入======"<<endl;
    return;
  }
  string msg=str.substr(idx+1);

  json js;
  js["id"]=g_currnetUser.GetId();
  js["name"]=g_currnetUser.GetName();
  js["groupid"]=groupid;
  js["msg"]=msg;
  js["time"]=getCurrentTime();
  js["msgid"]=GROUP_CHAT_MSG;


  string buffer=js.dump();
  int len=send(clientfd,buffer.c_str(),buffer.size(),0);
  if(len<0)
  {
    cerr<<"send group_chat_msg error"<<endl;
  }
}









// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}
