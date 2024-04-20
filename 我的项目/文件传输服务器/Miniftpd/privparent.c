#include "privparent.h"
#include "sysutil.h"
#include "privsock.h"
#include "tunable.h"

static void privop_pasv_get_data_sock(session_t *sess);
static void privop_pasv_active(session_t *sess);
static void privop_pasv_listen(session_t *sess);
static void privop_pasv_accept(session_t *sess);
int capset(cap_user_header_t hdrp, const cap_user_data_t datap)
{
	return syscall(__NR_capset,hdrp,datap);
}

void minimize_privilege()
{
	//给nobody进程相应的特权(特权最小化) 以便绑定20端口
	struct __user_cap_header_struct cap_header;
	struct __user_cap_data_struct cap_data;
	memset(&cap_header,0,sizeof(cap_header));
	memset(&cap_data,0,sizeof(cap_data));
	cap_header.version=_LINUX_CAPABILITY_VERSION_2;
	cap_header.pid=0;

	__u32 cap_mask=0;
	cap_mask |= (1 << CAP_NET_BIND_SERVICE);
	cap_data.effective=cap_data.permitted=cap_mask;
	cap_data.inheritable=0;
	capset(&cap_header,&cap_data);
}
void handle_parent(session_t *sess)
{
	minimize_privilege();
	while(1)
	{
		//接收由子进程发过来的命令信息
		char cmd;
		//read(sess->parent_fd,&cmd,1);
		cmd=priv_sock_get_cmd(sess->parent_fd); //接收命令以执行下面的函数
		switch(cmd)
		{
			case PRIV_SOCK_GET_DATA_SOCK:
				privop_pasv_get_data_sock(sess);
				break;
			case PRIV_SOCK_PASV_ACTIVE:
				privop_pasv_active(sess);
				break;
			case PRIV_SOCK_PASV_LISTEN:
				privop_pasv_listen(sess);
				break;
			case PRIV_SOCK_PASV_ACCEPT:
				privop_pasv_accept(sess);
				break;
		}

		//解析内部命令-----并不是ftp标准命令
		//处理内部命令
	}
}


static void privop_pasv_get_data_sock(session_t *sess)
{
	/*
	nobody进程接收PRIV_SOCK_GET_DATA_SOCK命令
	进一步接收一个整数  也就是port
	接收一个字符串      也就是ip
	fd=socket
	bind(20)
	connect(ip,port);

	OK
	send_fd
	BAD
	*/
	unsigned short port=priv_sock_get_int(sess->parent_fd);
	char ip[16]={0};
	priv_sock_recv_buf(sess->parent_fd,ip,sizeof(ip));
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=inet_addr(ip);
	int fd=tcp_client(20);//绑定20端口
	if(fd==-1)
	{
		priv_sock_send_result(sess->parent_fd,PRIV_SOCK_RESULT_BAD); //发送失败的应答
		return;
	}

	if(connect_timeout(fd,&addr,tunable_connect_timeout)<0)
	{
		priv_sock_send_result(sess->parent_fd,PRIV_SOCK_RESULT_BAD); //发送失败的应答
		close(fd);
		return ;
	}
	//发送成功的应答
	priv_sock_send_result(sess->parent_fd,PRIV_SOCK_RESULT_OK);
	priv_sock_send_fd(sess->parent_fd,fd);
	close(fd); 
}

static void privop_pasv_active(session_t *sess)
{
	int active;
	if(sess->pasv_listenfd!=-1)
		active=1;
	else
		active=0;

	priv_sock_send_int(sess->parent_fd,active);
}
static void privop_pasv_listen(session_t *sess)
{
	 sess->pasv_listenfd=tcp_server(NULL,0);//监听套接字
	 //0:表示 服务器在绑定的时候才确定绑定哪一个端口号
	 //可以通过调用getsockname()获取绑定的端口号
	 struct sockaddr_in localaddr;
	 socklen_t addrlen=sizeof(struct sockaddr);
	 if(getsockname(sess->pasv_listenfd,(struct sockaddr*)&localaddr,&addrlen)<0)//获取本地的地址信息 保存在localaddr中
		 ERR_EXIT("getsockname");
	 unsigned short port=ntohs(localaddr.sin_port);//将网络字节序转换为主机字节序
	 priv_sock_send_int(sess->parent_fd,(int)port);//发送给ftp服务进程
}
static void privop_pasv_accept(session_t *sess)
{
	int fd=accept_timeout(sess->pasv_listenfd,NULL,tunable_accept_timeout);//NULL表示：不需要客户端的地址信息
	
	close(sess->pasv_listenfd);//不论accept_timeout创建是否成功  都要关闭监听套接字pasv_listenfd
	sess->pasv_listenfd=-1;
	if(fd==-1)	
	{
		priv_sock_send_result(sess->parent_fd,PRIV_SOCK_RESULT_BAD); //发送失败的应答
		return ; 
	}
	priv_sock_send_result(sess->parent_fd,PRIV_SOCK_RESULT_OK); //发送失败的应答
	sess->data_fd=fd;
	priv_sock_send_fd(sess->parent_fd,fd); //发送数据套接字
	close(fd);  //直接关闭fd 因为nobody进程不直接与ftp客户端进行数据传输
}