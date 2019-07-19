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
#include<unistd.h>
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
	char text[150];
}MSG;
typedef struct sockaddr SA;
#define LEN_SMG sizeof(MSG)
	sqlite3 *db;
int process_login(int clientfd,MSG *msg);
int process_query(int clientfd,MSG *msg);
int process_history(int clientfd,MSG *msg);
void history_init(MSG *msg,char *buf);
int process_insert(int clientfd,MSG *msg);
int process_delete(int clientfd,MSG *msg);
int history_callback(void *arg, int ncolumn, char **f_value, char **f_name);
void get_time(const char *date);
int flags = 0;
void get_system_time(char* timedata)
{
	time_t t;
	struct tm *tp;

	time(&t);
	tp = localtime(&t);
	sprintf(timedata,"%d-%d-%d %d:%d:%d",tp->tm_year+1900,tp->tm_mon+1,\
			tp->tm_mday,tp->tm_hour,tp->tm_min,tp->tm_sec);
	return ;
}
int main(int argc,const char *argv[])
{
	int serverfd,clientfd;
	struct sockaddr_in serveraddr ,clientaddr;
	socklen_t len=sizeof(SA);
	int cmd;
	pid_t pid;
	MSG msg;
	ssize_t bytes;
	if(argc!=3)
	{
		printf("ip or port \n");
		return -1;
	}
   if(sqlite3_open("./staff.db",&db)!=SQLITE_OK)//打开数据库
   {
	   printf("%s\n",sqlite3_errmsg(db));
	   return -1;
   }
   if((serverfd=socket(AF_INET,SOCK_STREAM,0))<0)
   {
	   printf("socket fail");
	   return -1;
   }
   serveraddr.sin_family=AF_INET;
   serveraddr.sin_port=htons(atoi(argv[2]));
   serveraddr.sin_addr.s_addr=inet_addr(argv[1]);
   if(bind(serverfd,(SA*)&serveraddr,len)<0)
   {
     printf("bind fail\n");
	 return -1;
   }
   if(listen(serverfd,10)<0)
   {
	   printf("listen fail\n");
	   return -1;
   }
   while(1)
    {

	   if((clientfd=accept(serverfd,(SA*)&clientaddr,&len))<0)
	   {
		   perror("fail to accept");
		   continue;
	   }
	   pid=fork();
	   if(pid<0)
	   {
		   perror("fail to fork");
		   continue;
	   }
	   else if(pid==0) //接收客户端的请求处理过程
	   {
		   printf("*\n");
            //  close(serverfd);
		   while(1)
		   {
			   bytes=recv(clientfd,&msg,LEN_SMG,0);
			   printf("****\n");
			   if(bytes<=0)
				   break;
			   printf("****\n");
			   
			   switch(msg.type)
			   {
			   printf("****\n");
			   case Q:
				   process_query(clientfd,&msg);
				   break;
			   case L:
				   process_login(clientfd,&msg);
				   break;
			   case I:
				   process_insert(clientfd,&msg);
				   break;
			   case M:
				   process_modify(clientfd,&msg);
				   break;

			   case D:
				   process_delete(clientfd,&msg);
				   break;
			  case H:
				   process_history(clientfd,&msg);
				   break;
			   case E:
				   exit(0);
			   }
		   }
		   close(clientfd);
		   exit(1);
	   }
	   else
	   {
		   close(clientfd);
	   }
   }

   return 0;
}

// 帐号登录处理
int process_login(int clientfd,MSG *msg)
{
	printf("-------------------------------------------\n");
	char sql[150] = {0};
	char *errmsg;
	char **result;
	int n_row,n_column;
	printf("model: %#x-----usrname: %s---password: %s.\n",msg->model,msg->name,msg->password);
	sprintf(sql,"select * from userinfo where model=%d and name='%s' and password='%s';",msg->model,msg->name,msg->password);
	if(sqlite3_get_table(db,sql,&result,&n_row,&n_column,&errmsg) != SQLITE_OK)
	{
		printf("---****----%s.\n",errmsg);		
	}
	else
	{
		printf("----n_row-----%d,n_column-----%d.\n",n_row,n_column);		
		if(n_row==0)
		{
			strcpy(msg->text,"name or passwd failed.\n");
			send(clientfd,msg,sizeof(MSG),0);                   
		}
		else
		{
			strcpy(msg->text,"OK");
			send(clientfd,msg,sizeof(MSG),0);
		}
	}
	return		0;	
}

int process_query(int clientfd,MSG *msg)
{
	int i = 0,j = 0;
	char sql[150] = {0};
	char buf[128]={0};
	char **resultp;
	int n_row,n_column;
	char *errmsg;
	if(msg->select==1)
	{
		sprintf(sql,"select * from userinfo where name='%s';",msg->name);
	}
	else if(msg->select==2)
	{
		sprintf(sql,"select * from userinfo;");
	}

	if(sqlite3_get_table(db,sql,&resultp,&n_row,&n_column,&errmsg)!=SQLITE_OK)
	{
		printf("%s.\n",errmsg);
	}
	else
	{
		strcpy(msg->text,"OK");
		send(clientfd,msg,sizeof(MSG),0);
		printf("searching.....\n");
		printf("n_column:%d\tnrow:%d.\n",n_column,n_row);
		for(i = 0; i < n_column; i++)
		{
			printf("%-8s ",resultp[i]);
		}
		puts("");
		puts("=============================================================");
		int index = n_column;
		for(i = 0; i < n_row; i ++)
		{

			printf("%s    %s     %s     %s     %s     %s     %s     %s     %s     %s     %s.\n",resultp[index+n_column-11],resultp[index+n_column-10],\
					resultp[index+n_column-9],resultp[index+n_column-8],resultp[index+n_column-7],resultp[index+n_column-6],resultp[index+n_column-5],\
					resultp[index+n_column-4],resultp[index+n_column-3],resultp[index+n_column-2],resultp[index+n_column-1]);
			sprintf(msg->text,"%s    %s     %s     %s     %s     %s     %s     %s     %s     %s     %s.\n",resultp[index+n_column-11],resultp[index+n_column-10],\
					resultp[index+n_column-9],resultp[index+n_column-8],resultp[index+n_column-7],resultp[index+n_column-6],resultp[index+n_column-5],\
					resultp[index+n_column-4],resultp[index+n_column-3],resultp[index+n_column-2],resultp[index+n_column-1]);
			send(clientfd,msg,sizeof(MSG),0);
			usleep(1000);
			puts("=============================================================");
			index += n_column;
		}
		if(msg->select!= 1)
		{
			strcpy(msg->text,"over*");
			send(clientfd,msg,sizeof(MSG),0);
     	sprintf(buf,"管理员查询了所有用户");
	    history_init(msg,buf);
		}
		else
		{
     	sprintf(buf,"管理员查询了%s用户",msg->name);
	    history_init(msg,buf);
		}
		sqlite3_free_table(resultp);
		printf("sqlite3_get_table successfully.\n");
	}
}
int process_insert(int clientfd,MSG *msg)
{
	char sql[150] = {0};
	char buf[128] = {0};
	char *errmsg;
	recv(clientfd,msg,LEN_SMG,0);
	printf("%d\t %d\t %s\t %s\t  %d\t %d.\n",msg->number,msg->model,msg->name,msg->password,\
			msg->age,msg->salary);
	sprintf(sql,"%d\t %d\t %s\t %s\t %d\n  %d\t\n",msg->number,msg->model,msg->name,msg->password,\
			msg->age, msg->salary);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK)
	{
		printf("----------%s.\n",errmsg);
		strcpy(msg->text,"failed");
		send(clientfd,msg,sizeof(MSG),0);
		return -1;
	}
	else
	{
		strcpy(msg->text,"OK");
		send(clientfd,msg,sizeof(msg),0);
		printf("%s insert  success.\n",msg->name);
	}
	sprintf(buf,"管理员添加了%s用户",msg->name);
	history_init(msg,buf);
    return 0;
}

int process_modify(int clientfd,MSG *msg)
{
	int nrow,ncolumn;
	char *errmsg, **resultp;
	char sql[150] = {0};	
	char historybuf[128] = {0};
	switch (msg->select)
	{
	case '1':
		sprintf(sql,"update userinfo set name='%s' where staffno=%d;",msg->name, msg->number);
		sprintf(historybuf,"管理员修改工号为%d的用户名为%s",msg->number,msg->name);
		break;
	case '2':
		sprintf(sql,"update userinfo set age='%d' where staffno=%d;",msg->age, msg->number);
		sprintf(historybuf,"管理员修改工号为%d的年龄为%d",msg->number,msg->age);
		break;
	case '5':
		sprintf(sql,"update userinfo set salary='%d' where staffno=%d;",msg->salary, msg->number);
		sprintf(historybuf,"管理员修改工号为%d的薪资为%d",msg->number,msg->salary);
		break;
	case '4':
		sprintf(sql,"update userinfo set password='%s' where staffno=%d;",msg->password, msg->number);
		sprintf(historybuf,"管理员修改工号为%d的密码为%s",msg->number,msg->password);
		break;
	case '3':
		sprintf(sql,"update userinfo set call='%s' where staffno=%d;",msg->call, msg->number);
		sprintf(historybuf,"管理员修改工号为%d的电话为%s",msg->number,msg->call);
		break;
	}
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("%s.\n",errmsg);
		sprintf(msg->text,"数据库修改失败！%s", errmsg);
	}
	else
	{
		printf("the database is updated successfully.\n");
		sprintf(msg->text, "ok ");
		history_init(msg,historybuf);
	}
	send(clientfd,msg,sizeof(MSG),0);

	printf("------%s.\n",historybuf);
	return 0;
}

int process_delete(int clientfd,MSG *msg)
{
	char sql[150] = {0};
	char buf[128] = {0};
	char *errmsg;
	printf("msg->number :%d\t msg->name: %s\n",msg->number,msg->name);
sprintf(sql,"delete from userinfo where number=%d and name='%s';",msg->number,msg->name);	
if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK)
{
	printf("----------%s.\n",errmsg);
	strcpy(msg->text,"failed");
	send(clientfd,msg,sizeof(MSG),0);
	return -1;
}
else
{
	strcpy(msg->text,"OK");
	send(clientfd,msg,sizeof(msg),0);
	printf("%s delete %s success.\n",msg->name,msg->name);
}
sprintf(buf,"管理员删除了%s用户",msg->name);
history_init(msg,buf);


}
	
	
	//封装历史记录  
void history_init(MSG *msg,char *buf)
{
	int n_row,n_column;
	char *errmsg, **resultp;
	char sqlhistory[150] = {0};
	char timedata[128] = {0};

	get_system_time(timedata);

	sprintf(sqlhistory,"insert into historyinfo values\
			('%s','%s','%s');",timedata,msg->name,buf);
	if(sqlite3_exec(db,sqlhistory,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s.\n",errmsg);
		printf("insert historyinfo failed.\n");
	}
	else
	{
		printf("insert historyinfo success.\n");
	}
}
//历史查询处理
int process_history(int clientfd,MSG *msg)
{
	char sql[150] = {0};
	char *errmsg;
	flags = clientfd; //临时保存通信的文件描述符
	sprintf(sql,"select * from historyinfo;");
	if(sqlite3_exec(db,sql,history_callback,(void *)msg,&errmsg) != SQLITE_OK)
	{
		printf("%s.\n",errmsg); 	
	}
	else
	{
		printf(" ok done.\n");
	}
	//通知对方查询结束了 
	strcpy(msg->text,"over*");
	send(clientfd,msg,sizeof(MSG),0);
	flags=0;
}
int history_callback(void *arg, int ncolumn, char **f_value, char **f_name)
{
	int i = 0;
	MSG *msg= (MSG *)arg;
	int clientfd = flags;

	if(flags == 0){
		for(i = 0; i < ncolumn; i++){
			printf("%-11s", f_name[i]);
		}
		putchar(10);
		flags = 1;
	}

	for(i = 0; i < ncolumn; i+=3)
	{
		printf("%s-%s-%s",f_value[i],f_value[i+1],f_value[i+2]);
		sprintf(msg->text,"%s---%s---%s.\n",f_value[i],f_value[i+1],f_value[i+2]);
		send(clientfd,msg,sizeof(MSG),0);
		usleep(1000);
	}
	puts("");

	return 0;
}
