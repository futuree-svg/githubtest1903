#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sqlite3.h> 
#include <signal.h>
#include <time.h>
#define A 0
#define B 1
#define E 0x3
#define Q 0x4
#define M 0x12
#define I 0x13
#define D 0x14
#define H 0x15
#define L 0x5
typedef struct 
{
	int model;//模式
	int type;//消息类型
	char name[32];//用户名
	char password[6];//密码
	int  number;//工号
	int age;
	int select;
	int salary;
	char call[11];
	char text[32];
}MSG,*MSG_t;
#define LEN_SMG sizeof(MSG)
typedef struct sockaddr SA;

void do_login(int sockfd,MSG *msg,int cmd);
void do_history(int sockfd,MSG *msg);
void do_delete(int sockfd,MSG *msg);
void do_insert(int sockfd,MSG *msg);
void do_query(int sockfd,MSG *msg);
void do_modify(int sockfd,MSG *msg);
void do_user_query(int sockfd,MSG *msg);
int main(int argc, const char *argv[])
{

	int sockfd;
	struct sockaddr_in serveraddr;
	socklen_t len=sizeof(SA);
	int cmd;
	MSG *msg;
	char clean[M]={0};
	if(argc!=3)
	{
		printf(" ./ ip or port\n");
		return -1;
	}

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		printf("fail to socket");
		return -1;
	}
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr=inet_addr(argv[1]);
	if(connect(sockfd,(SA*)&serveraddr,len)<0)
	{
		printf("fail to connect");
		return -1;
	}
BEF:
	while(1)//一级的界面
	{
		puts("**********************************************");
		puts("1.管理员模式        2.用户模式         3.退出");
		puts("**********************************************");
		printf("input cmd>>>");
		scanf("%d",&cmd);
		printf("----------\n");
		switch(cmd)//根据不同的输入命令执行不同的行为
		{
		case 1:
			printf("****\n");
			do_login(sockfd,msg,1);
			goto NEXT1;
			break;
		case 2: 
			do_login(sockfd,msg,2);
			goto NEXT2;
			break;
		case 3:
			goto EXIT;
		default:
			puts("input error");
			break;
		}				
	}
EXIT:  
	msg->type=E;
	send(sockfd,msg,LEN_SMG,0);
	close(sockfd);  //客户端退出的处理   
	exit(0);


	//管理员登录
NEXT1:
	printf("亲爱的管理员，欢迎你登录员工管理系统！\n");
	while(1)
	{
		puts("**********************************************");
		puts("**1查询 2.修改  3.添加用户 4.删除用户5.查询历史记录  6.退出");
		puts("**********************************************");
	printf("input cmd>>>");
    scanf("%d",&cmd);
	switch(cmd)
	{
	   case 1:
           do_query(sockfd,msg);  
		   break;
	   case 2:
           do_modify(sockfd,msg);  
		   break;
	   case 3:
           do_insert(sockfd,msg);  
		   break;
	   case 4:
           do_delete(sockfd,msg);  
		   break;
	   case 5:
           do_history(sockfd,msg);  
		   break;
	   case 6:
		   goto BEF;
	   default:
		   puts("cmd error");
		   break;
	}
}



//用户登录
NEXT2:
  printf("亲爱的用户，欢迎你登录员工管理系统！\n");
  while(1)
  {
  puts( "*******************************************************");
  puts( "********* 1：查询  	2：修改		3：退出	 ********");
  puts("********************************************************");
  puts("input cmd");
  scanf("%d",&cmd);
  switch(cmd)
  {
	  case 1:
          do_user_query(sockfd,msg);
		  break;
	  case 2: 
         do_modify(sockfd,msg);
		  break;
	  case 3:
		  goto BEF;
	  default:
		   puts("cmd error");
		   break;
  }
 }
close(sockfd);
return 0;
}
//帐号密码输入
void do_login(int sockfd,MSG *msg,int cmd)
{
	puts("input name>>>");
	scanf("%s",msg->name);
	puts("input password>>>");
	scanf("%s",msg->password);
	msg->model=cmd-1;
    msg->type=L;
    send(sockfd,msg,LEN_SMG,0);
	recv(sockfd,msg,LEN_SMG,0);
	if(strncmp(msg->text,"OK",2)==0)
	{   
		puts("Login ok!");
		return ;
	}
	else
	{
		puts("Login fail!");
		return ;
	}
} 

void do_user_query(int sockfd,MSG *msg)
{
          msg->type=Q;
		  msg->select=1;
		  send(sockfd,msg,LEN_SMG,0);
		  recv(sockfd,msg,LEN_SMG,0);
		  if(strncmp(msg->text,"OK",2)==0)
		  {
			  recv(sockfd,msg,LEN_SMG,0);
			  printf("模式 工号 姓名  密码     年龄  薪资   电话\n");
			  printf("---------------------------------------------\n");
			  printf("%s. \n",msg->text);
			  printf("--------------------------------------------\n");
		  }
		  else
		  {
			  printf("user query fail\n");
		  }

}
//管理员查询
void do_query(int sockfd,MSG *msg)
{
	int cmd;
	while(1)                                                                                                                                                                                                                                                                                                                                                                                                                     
	{
		puts("**********************************************");
		puts("**1.按人名查找 2.查找所有  3.退出");
		puts("**********************************************");
	}
	printf("input cmd>>>");
    scanf("%d",&cmd);
	switch(cmd)
	{
      case 1:
          msg->select=1;
		  printf("请输入要查找的姓名：");
		  scanf("%s",msg->name);
		  send(sockfd,msg,LEN_SMG,0);
		  recv(sockfd,msg,LEN_SMG,0);
		  if(strncmp(msg->text,"OK",2)==0)
		  {
			  recv(sockfd,msg,LEN_SMG,0);
			  printf("模式 工号 姓名  密码     年龄  薪资   电话\n");
			  printf("---------------------------------------------\n");
			  printf("%s.  \n",msg->text);
			  printf("--------------------------------------------\n");
		  }
		  else
		  {
			  puts("query  fail!");	
		  }

			  break;
	 case 2:
		  msg->select=2;
		  int i=0;
		  send(sockfd,msg,LEN_SMG,0);
		  recv(sockfd,msg,LEN_SMG,0);
		  if(strncmp(msg->text,"OK",2)==0)
		  {
			  printf("模式 工号 姓名  密码     年龄  薪资   电话\n");
			  printf("-------------------------------------------\n");
			  while(1)
			  {
				  recv(sockfd,msg,LEN_SMG,0);
				  if(strncmp(msg->name,"over",4)==0)//记录结束跳出循环
				  {
					  break;
				  }
				  printf("%s. \n",msg->text);
                  printf("--------------------------------------------\n");
			  }
		  }  
			  break;
	case 3:
		  break;
	default:
		  printf("cmd error\n");
		  break;
	}
}
//管理员、用户修改
void do_modify(int sockfd,MSG *msg)
{
	int cmd;
  msg->type=M;
  printf("请输入您要修改的工号：");
  scanf("%d",&msg->number);
  puts(" *******************请输入要修改的选项********************");
  puts( " ******	1：姓名	  2：年龄	  3：电话  ******");
  puts( "******	4：密码	 5: 薪资      6：退出  *******");
  puts(  "*************************************************************");
  printf("input cmd");
	 scanf("%d",&cmd);
	 msg->select=cmd;
	 switch(cmd)
	 {
	 case 1:
		 printf("input name:");
		 scanf("%s",msg->name);
		 break;
	 case 2:
		 printf("input age:");
		 scanf("%d",&msg->age);
		 break;
	 case 3:
		 printf("input call:");
		 scanf("%s",msg->call);
		 break;
	 case 4:
		 printf("input password:");
		 scanf("%s",msg->password);
		 break;
	 case 5:
		 printf("input salary:");
		 scanf("%d",&msg->salary);
		 break;
	 case 6:
		 break;
	default:
		  puts("cmd error");
		  break;
	 }
		 send(sockfd,msg,LEN_SMG,0);
		 recv(sockfd,msg,LEN_SMG,0); 
		 if(strncmp(msg->text,"OK",2)==0)
		 {
			 puts("  modify   ok!\n");
		 }
		 else
		 {
			 puts("modify  fail!");

		 }

}
//管理员添加
//
//
void do_insert(int sockfd,MSG *msg)
{
	msg->type=I;
	printf("请输入工号：");
	scanf("%d",&msg->number);
	printf("工号信息一旦输入无法更改，请确认输入是否正确（y/n）");
	if(getchar()==121)
	{
		printf("请输入用户名：");
		scanf("%s",msg->name);
		printf("请输入用户密码：");
		scanf("%s",msg->password);
		printf("请输入年龄：");
		scanf("%d",&msg->age);
		printf("请输入薪资");
		scanf("%d",&msg->salary);
		printf("请输入电话号码：");
		scanf("%s",msg->call);
		printf("是否为管理员（y/n）");
		if(getchar()==121)
		{
			msg->model=A;
			send(sockfd,msg,LEN_SMG,0);//用户名密码发给服务器
			recv(sockfd,msg,LEN_SMG,0); //等待服务器回应
			if(strncmp(msg->text,"OK",2)==0)
			{
				puts(" insert ok! continue(y/n)");
				if(getchar()==121)
				{
					do_insert(sockfd,msg);
				}
				else
                  return;
			}
			else
			{
				printf("insert fail\n");
			}
		}
		else if(getchar()==110)
		{
			msg->model=B;
			send(sockfd,msg,LEN_SMG,0);//用户名密码发给服务器
			recv(sockfd,msg,LEN_SMG,0); //等待服务器回应
			if(strncmp(msg->text,"OK",2)==0)
			{
				puts(" insert ok! continue(y/n)");
				if(getchar()==121)
				{
					do_insert(sockfd,msg);
				}
				else
				 return;
			}
			else
			{
				printf("insert fail\n");
			}
		}
		else
		{
			printf("input fail\n");
			return;
		}
	}
else
	return;	
}

//管理员删除
void do_delete(int sockfd,MSG *msg)
{
	msg->type=D;
printf("请输入要删除的用户工号;");
scanf("%d",&msg->number);
printf("请输入要删除的用户名：");
scanf("%s",msg->name);
send(sockfd,msg,LEN_SMG,0);//用户名密码发给服务器
recv(sockfd,msg,LEN_SMG,0);//等待服务器回应
if(strncmp(msg->name,"NO",2)==0)
{
printf("数据库修改成功，用户已经删除\n");
	return ;
}
else
{
	puts("delete fail!");
	return ;
}
}
//历史查询
//
//
void do_history(int sockfd,MSG *msg)
{
	msg->type=H;
	int i=0;
	send(sockfd,msg,LEN_SMG,0);
	recv(sockfd,msg,LEN_SMG,0);
	if(strncmp(msg->text,"OK",2)==0)//printf有问题
	{

		while(1)
		{
			recv(sockfd,msg,LEN_SMG,0);//等待服务器发送的历史记录条目
			if(strncmp(msg->text,"over",4)==0)//记录结束跳出循环
			{
				break;
			}
		    printf("msg->txet: %s",msg->text);
		}
		puts("------------------------------------");
		puts("History ok!");
		return;
	}
	else
	{
		puts("History fail!");	
		return;
	}
}

