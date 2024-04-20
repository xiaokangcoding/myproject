#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>
using namespace std;
#include<mysql/mysql.h>

int main()
{
	printf("Content-Type:text/html\n\n");
	MYSQL mysql;
	mysql_init(&mysql);
	MYSQL *conn_ptr=mysql_real_connect(&mysql,"localhost","root","123","test",3306,NULL,0);
	/*if(conn_ptr!=NULL)
		printf(" Connect Mysql successfully!\n");
	else
	{
		printf("<p> Connect Mysql failed!\n");
		exit(1);
	}*/
	char user[20]={0};
	char pwd[20]={0};
//	char *p=getenv("QUERY_STRING");
	int len=atoi(getenv("CONTENT_LENGTH"));
	char *p=(char *)malloc(len+1);
	fgets(p,len+1,stdin);
	int i=0;
	while(*p!='=')
		p++;
	p++;//跳过=
	while(*p!='&')
	{
		user[i++]=*p;
		p++;
	}
	p++;//跳过 &
	while(*p!='=')
		p++;
	p++;
	i=0;
	while(*p!='\0')
	{
		pwd[i++]=*p;
		p++;
	}

	char sql[100]={0};
	sprintf(sql,"insert into user values(null,'%s','%s');",user,pwd);		
	int ret=mysql_query(&mysql,sql);//成功返回0
//	if(!ret)
//	{
//		printf("Register successfully!\n");
//	}
	mysql_close(&mysql);

	FILE *fp=fopen("/root/muduo/03/jmuduo/muduo/net/myhttp/index.html","r");
	if(fp==NULL)
	{
		printf("<h1> NOt Found 404 </h1>");
		exit(1);
	}
	char buf[128];
	while(!feof(fp))//feof() 文件结束：返回非0值，文件未结束，返回0值
	{	
		fgets(buf,128,fp);
		printf(buf);
	}
	fclose(fp);
	return 0;
}
