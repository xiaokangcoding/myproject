#include<stdio.h>
#include<stdlib.h>
#include<mysql/mysql.h>
#include<string.h>

int main()
{
	printf("Content-Type:text/html\n\n");
	MYSQL mysql;
	mysql_init(&mysql);
	MYSQL *conn_ptr;
	conn_ptr=mysql_real_connect(&mysql,"localhost","root","123","test",0,NULL,0);
	if(conn_ptr)
		//		printf("<p> Connect Mysql Successfully!\n");
		;
	else
	{
		printf("<h1 align='center'> Connect Mysql failed!</h1>");
		exit(1);
	}
	char user[20]={0};
	char pwd[20]={0};
	//	char *p=getenv("QUERY_STRING");
	int len=atoi(getenv("CONTENT_LENGTH"));
	char *p=(char*)malloc(len+1);
	fgets(p,len+1,stdin);
	int i=0;
	while(*p!='=')
		p++;
	p++; //skip =
	while(*p!='&')
	{
		user[i++]=*p;
		p++;
	}
	p++;//skip &
	while(*p!='=')
	{
		p++;
	}
	p++;//skip =
	i=0;
	while(*p!='\0')
	{
		pwd[i++]=*p;
		p++;
	}


	char select[100];
	MYSQL_ROW row;
	MYSQL_RES *res_ptr;

	sprintf(select,"select pwd from user where name='%s'",user);
	int ret=mysql_query(&mysql,select);
	if(ret)
	{
		printf("<h1> query error!</h1>");
		exit(1);
	}
	res_ptr=mysql_store_result(&mysql);
	if(res_ptr!=NULL)
		row=mysql_fetch_row(res_ptr);
	else
		exit(1);
	if(strcmp(row[0],pwd)==0)
	{
		FILE *fp=fopen("/root/muduo/03/jmuduo/muduo/net/myhttp/BigIntCal.html","r");
		if(NULL==fp)
		{
			printf("<h1>Not Found 404</h1>");
			exit(1);
		}
		char buf[128];
		while(!feof(fp))
		{
			fgets(buf,128,fp);
			printf(buf);
		}
		fclose(fp);
	}
	else
	{
		printf("<h1 align='center' color='red'>username or user password error!!!</h1>");
	}
	mysql_close(&mysql);
	return 0;
}
