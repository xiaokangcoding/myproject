#include "session.h"
#include "ftpproto.h"
#include "privparent.h"
#include "privsock.h"
#include "sysutil.h"
void begin_session(session_t *sess)
{
	//用于进程之间的通信
/*	int sockfds[2];
	int ret=socketpair(PF_UNIX,SOCK_STREAM,0,sockfds);
	if(ret==-1)
		ERR_EXIT("socketpair");
		*/
	activate_oobinline(sess->ctrl_fd); //带外数据的接受
	priv_sock_init(sess);
	//再创建进程来 nobody进程   ftp服务进程
	pid_t pid;
	pid=fork();
	if(pid<0)
		ERR_EXIT("fork");
	if(pid==0) //ftp服务进程(直接与ftp客户端进行交互  接受ftp客户端发送过来的请求 并且对这些请求进行处理 最后响应给ftp客户端)------主要处理ftp协议相关的通信细节--ftpproto.c
	{			//ftp服务进程 不仅要处理控制连接(控制连接:主要负责接受ftp客户端发送过来的命令请求) 
				//也要处理数据连接(数据连接:用于传输数据 比如说:文件传输 目录列表的传输)
		priv_sock_set_child_context(sess);
		//close(sockfds[0]);//ftp服务进程既可以读 也可以写
		//sess->child_fd=sockfds[1];
		handle_child(sess);
	}
	else if(pid>0) //nobody进程------父进程------privparent.c
	{	
		//将当前进程改成nobody进程
		struct passwd *pw;
		pw=getpwnam("nobody");
		if(pw==NULL)
			return;
		//如果先改用户id 就很有可能没有权限去修改组id
		if(setegid(pw->pw_gid)<0)//先改组id 再改用户id
			ERR_EXIT("setegid");
		if(seteuid(pw->pw_uid)<0)
			ERR_EXIT("seteuid");
		//nobody进程接收ftp服务进程发过来的一些命令，来协助ftp服务进程完成任务 
		//即: 辅助ftp服务进程建立与ftp客户端之间的数据连接
		//close(sockfds[1]);//nobody进程既可以读 也可以写
		//sess->parent_fd=sockfds[0];
		priv_sock_set_parent_context(sess);
		handle_parent(sess);
	}
	//priv_sock_close(sess);
}

