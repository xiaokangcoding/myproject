#include "privsock.h"
#include "sysutil.h"
/////////////////////////////////////////////////////////////////
void priv_sock_init(session_t *sess)
{
	int sockfds[2];
	int ret=socketpair(PF_UNIX,SOCK_STREAM,0,sockfds);
	if(ret==-1)
		ERR_EXIT("socketpair");
	sess->child_fd=sockfds[1];
	sess->parent_fd=sockfds[0];
}
void priv_sock_close(session_t *sess)
{
	if(sess->child_fd!=-1)   
	{
		close(sess->child_fd);
		sess->child_fd=-1;
	}
	if(sess->parent_fd!=-1)    
	{
		close(sess->parent_fd);
		sess->parent_fd=-1;
	}

}
void priv_sock_set_parent_context(session_t *sess)
{
	if(sess->child_fd!=-1)   //父进程不需要子进程的fd
	{
		close(sess->child_fd);
		sess->child_fd=-1;
	}
}
void priv_sock_set_child_context(session_t *sess)
{
	if(sess->parent_fd!=-1)    //子进程不需要父进程的fd
	{
		close(sess->parent_fd);
		sess->parent_fd=-1;
	}
}
///////////////////////////////////////////////////////////////////////
//ftp进程向nobody进程发送命令 子进程---> 父进程
void priv_sock_send_cmd(int fd, char cmd)
{
	int ret=writen(fd,&cmd,sizeof(cmd));
	if(ret!=sizeof(cmd))
	{
		fprintf(stderr,"priv_sock_send_cmd");
		exit(EXIT_FAILURE);
	}
}
//nobody进程获取从ftp进程发来的命令
char priv_sock_get_cmd(int fd)
{
   char result;
   int res=readn(fd,&result,sizeof(result));
   if(res==0)
   {
      fprintf(stderr,"FTP process exit!");
      exit(1); //nobody 进程也退出
   }
   if(res!=sizeof(result))
   {
      fprintf(stderr,"priv_sock_get_cmd");
      exit(EXIT_FAILURE);
   }
   return result;
}
//nobody进程向ftp进程发送命令应答
void priv_sock_send_result(int fd, char res)
{
	int ret=writen(fd,&res,sizeof(res));
	if( ret!=sizeof(res) )
	{
		fprintf(stderr,"priv_sock_send_result");
		exit(EXIT_FAILURE);
	}
}
//ftp服务进程获取从nobody进程发送命令应答
char priv_sock_get_result(int fd)
{
	char result;
	int ret=readn(fd,&result,sizeof(result));
	if(ret!=sizeof(result))
	{
		fprintf(stderr,"priv_sock_get_result");
		exit(EXIT_FAILURE);
	}
	return result;
}

void priv_sock_send_int(int fd, int the_int)
{
	int ret=writen(fd,&the_int,sizeof(the_int));
	if(ret!=sizeof(the_int) )
	{
		fprintf(stderr,"priv_sock_send_int");
		exit(EXIT_FAILURE);
	}
}
int priv_sock_get_int(int fd)
{
	int the_int;
	int res=readn(fd,&the_int,sizeof(the_int));
	if(res==0)
	{
		fprintf(stderr,"FTP 服务进程 exit!");
		exit(1); //nobody 进程也退出
	}
	if(res!=sizeof(the_int))
	{
		fprintf(stderr,"priv_sock_get_int");
		exit(EXIT_FAILURE);
	}
	return the_int;
}
//发送一个字符串
void priv_sock_send_buf(int fd, const char *buf, unsigned int len)
{
	priv_sock_send_int(fd,(int)len);//先发送一个长度
	int ret=writen(fd,buf,len);
	if(ret!=(int)len)
	{
		fprintf(stderr,"priv_sock_send_buf");
		exit(EXIT_FAILURE);
	}
}
//接收一个字符串
void priv_sock_recv_buf(int fd, char *buf, unsigned int len)
{
	unsigned int recv_len=priv_sock_get_int(fd);//先接收一个长度
	if(recv_len>len)
	{
		fprintf(stderr,"priv_sock_recv_buf");
		exit(EXIT_FAILURE);
	}
	int ret=readn(fd,buf,recv_len);
	if((unsigned int)ret!=recv_len)
	{
		fprintf(stderr,"priv_sock_recv_buf");
		exit(EXIT_FAILURE);
	}
}

void priv_sock_send_fd(int sock_fd, int fd)
{
	send_fd(sock_fd,fd);
}

int priv_sock_recv_fd(int sock_fd)
{
	return recv_fd(sock_fd);
}