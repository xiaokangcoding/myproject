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
	if(sess->child_fd!=-1)   //�����̲���Ҫ�ӽ��̵�fd
	{
		close(sess->child_fd);
		sess->child_fd=-1;
	}
}
void priv_sock_set_child_context(session_t *sess)
{
	if(sess->parent_fd!=-1)    //�ӽ��̲���Ҫ�����̵�fd
	{
		close(sess->parent_fd);
		sess->parent_fd=-1;
	}
}
///////////////////////////////////////////////////////////////////////
//ftp������nobody���̷������� �ӽ���---> ������
void priv_sock_send_cmd(int fd, char cmd)
{
	int ret=writen(fd,&cmd,sizeof(cmd));
	if(ret!=sizeof(cmd))
	{
		fprintf(stderr,"priv_sock_send_cmd");
		exit(EXIT_FAILURE);
	}
}
//nobody���̻�ȡ��ftp���̷���������
char priv_sock_get_cmd(int fd)
{
   char result;
   int res=readn(fd,&result,sizeof(result));
   if(res==0)
   {
      fprintf(stderr,"FTP process exit!");
      exit(1); //nobody ����Ҳ�˳�
   }
   if(res!=sizeof(result))
   {
      fprintf(stderr,"priv_sock_get_cmd");
      exit(EXIT_FAILURE);
   }
   return result;
}
//nobody������ftp���̷�������Ӧ��
void priv_sock_send_result(int fd, char res)
{
	int ret=writen(fd,&res,sizeof(res));
	if( ret!=sizeof(res) )
	{
		fprintf(stderr,"priv_sock_send_result");
		exit(EXIT_FAILURE);
	}
}
//ftp������̻�ȡ��nobody���̷�������Ӧ��
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
		fprintf(stderr,"FTP ������� exit!");
		exit(1); //nobody ����Ҳ�˳�
	}
	if(res!=sizeof(the_int))
	{
		fprintf(stderr,"priv_sock_get_int");
		exit(EXIT_FAILURE);
	}
	return the_int;
}
//����һ���ַ���
void priv_sock_send_buf(int fd, const char *buf, unsigned int len)
{
	priv_sock_send_int(fd,(int)len);//�ȷ���һ������
	int ret=writen(fd,buf,len);
	if(ret!=(int)len)
	{
		fprintf(stderr,"priv_sock_send_buf");
		exit(EXIT_FAILURE);
	}
}
//����һ���ַ���
void priv_sock_recv_buf(int fd, char *buf, unsigned int len)
{
	unsigned int recv_len=priv_sock_get_int(fd);//�Ƚ���һ������
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