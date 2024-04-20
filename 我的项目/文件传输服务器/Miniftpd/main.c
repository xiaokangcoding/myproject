#include"common.h"
#include"session.h"
#include"sysutil.h"
#include "str.h"
#include "ftpproto.h"
#include "parseconf.h"
#include "tunable.h"
#include "hash.h"
/*
//该结构体保存一些当前会话所需要的一些变量
typedef struct session
{
	//控制连接所需要的一些变量
	uid_t uid;
	int ctrl_fd;//控制连接的套接字 已连接的套接字 conn
	char cmdline[MAX_COMMAND_LINE];
	char cmd[MAX_COMMAND];
	char arg[MAX_ARG];
	//数据连接
	struct sockaddr_in *port_addr;
	int pasv_listenfd;//判断是否处于pasv_active()状态---监听套接字
	
	//数据连接套接字
	int data_fd;
	int data_process;
	//限速
	unsigned int bw_upload_rate_max;//上传的最大速率
	unsigned int bw_download_rate_max;//下载的最大速率
	long bw_transfer_start_sec;//开始的传输时间(s)
	long bw_transfer_start_usec;//开始的传输时间(us)微妙
	//父子进程的通道
	int parent_fd,child_fd;
	//FTP协议状态
	int is_ascii;
	long long restart_pos;//用于断点续传
	char * rnfr_name; 
	int abor_received;
	//连接数的限制
	unsigned int num_clients;
	unsigned int num_this_ip;
}session_t;
*/
extern session_t *p_sess;
static unsigned int s_children;
void check_limits(session_t *sess);

static hash_t *s_ip_count_hash;
static hash_t *s_pid_ip_hash;

unsigned int hash_func(unsigned int buckets, void*key);
unsigned int handle_ip_count(void *ip);
void drop_ip_count(void *ip);

void handle_sigchld(int sig);
int main()
{
	parseconf_load_file(MINIFTP_CONF);
	printf("tunable_pasv_enable=%d\n",tunable_pasv_enable);
	printf("tunable_port_enable=%d\n",tunable_port_enable);

	printf("tunable_listen_port=%u\n",tunable_listen_port);
	printf("tunable_max_clients=%u\n",tunable_max_clients);
	printf("tunable_max_per_ip=%u\n",tunable_max_per_ip);
	printf("tunable_accept_timeout=%u\n",tunable_accept_timeout);
	printf("tunable_connect_timeout=%u\n",tunable_connect_timeout);
	printf("tunable_idle_session_timeout=%u\n",tunable_idle_session_timeout);
	printf("tunable_data_connection_timeout=%u\n",tunable_data_connection_timeout);
	printf("tunable_local_umask=0%o\n",tunable_local_umask);
	printf("tunable_upload_max_rate=%u\n",tunable_upload_max_rate);

	printf("tunable_download_max_rate=%u\n",tunable_download_max_rate);
	if(tunable_listen_address==NULL)
		printf("tunable_listen_address=NULL\n");
	else
		printf("tunable_listen_address=%s\n",tunable_listen_address);
	//printf("tunable_port_enable=%d\n",tunable_port_enable);
	//首先Miniftp服务器的启动必须是root用户---首先判断是否是root用户启动的程序
	if(getuid()!=0)//getuid() 返回当前进程的真实用户ID
	{
		fprintf(stderr,"Miniftp: must be started as root\n");
		exit(EXIT_FAILURE);
	}
	
	int listenfd=tcp_server(tunable_listen_address,tunable_listen_port);//NULL: 给tcp_server()函数的第一个参数传NULL 表示用本机任意ip地址
	int conn;
	//会话变量
	session_t sess=
	{
		//控制连接
		0,-1,
		"","","",
		//数据连接 //数据连接套接字
		NULL, -1,-1,0,
		//限速
		0,0,0,0,
		//父子进程的通道
		-1,-1,
		//FTP协议状态
		0,0,NULL,0,
		//连接数的限制
		0,0
	};
	p_sess=&sess;
	sess.bw_upload_rate_max=tunable_upload_max_rate;
	sess.bw_download_rate_max=tunable_download_max_rate;

	s_ip_count_hash=hash_alloc(256,hash_func);//创建一个哈希表
	s_pid_ip_hash=hash_alloc(256,hash_func);//创建一个哈希表

	struct sockaddr_in addr;
	
	signal(SIGCHLD,handle_sigchld);//在当前父进程 安装信号处理程序
	while(1)
	{
		//第二个参数表示对等方的地址
		conn=accept_timeout(listenfd,&addr,0);//0: 表示不采用超时的方式接受客户端的连接
		//printf("%s %d\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
		if(conn==-1)
			ERR_EXIT("accept_timeout");
		//如果成功的话 返回一个已连接套接字

		unsigned int ip = addr.sin_addr.s_addr;
		printf("ip=%d\n",ip);		
		ip=ntohl(ip);
		printf("/////////////////\n");
		printf("ip=%d\n",ip);		
		s_children++;
		sess.num_clients=s_children;
		sess.num_this_ip=handle_ip_count(&ip);

		pid_t pid=fork();//开始创建进程

		//主进程接受客户端的连接 子进程用来创建一个会话
		if(pid==0)
		{
			close(listenfd);
			sess.ctrl_fd=conn;
			check_limits(&sess);//连接数的判定   最大连接数  以及每ip对应的连接数

			signal(SIGCHLD,SIG_IGN);
			begin_session(&sess);//启动一个会话  ---- 包含两个进程
		}
		else if(pid>0)
		{
			hash_add_entry(s_pid_ip_hash,&pid,sizeof(pid),&ip,sizeof(unsigned int));
			close(conn);
		}
		else
		{
			s_children--;
			ERR_EXIT("fork");
		}
	}
	return 0;
}

//连接数的判定
void check_limits(session_t *sess)
{
	if (tunable_max_clients>0 && sess->num_clients > tunable_max_clients)
	{
		ftp_reply(sess,FTP_TOO_MANY_USERS,"There are too many connected users, please try later.");
		exit(EXIT_FAILURE);
	}

	//每个ip对应的连接数的判断
	if (tunable_max_per_ip>0 && sess->num_this_ip > tunable_max_per_ip)
	{
		ftp_reply(sess,FTP_IP_LIMIT," There are too many connections from your internet address.");
		exit(EXIT_FAILURE);
	}
}

void handle_sigchld(int sig)
{
	// 当一个客户端退出的时候，那么该客户端对应ip的连接数要减1，
	// 处理过程是这样的，首先是客户端退出的时候，
	// 父进程需要知道这个客户端的ip，这可以通过在s_pid_ip_hash查找得到，
	pid_t pid;
	while ( (pid=waitpid(-1,NULL,WNOHANG))>0)
	{
		--s_children;
		unsigned int *ip=(unsigned int*)hash_lookup_entry(s_pid_ip_hash,&pid,sizeof(pid));
		if(ip==NULL)
			return ;
		drop_ip_count(ip);
		hash_free_entry(s_pid_ip_hash,&pid,sizeof(pid));
	}
}

unsigned int hash_func(unsigned int buckets, void*key)
{
	unsigned int *number = (unsigned int*)key;
	return (*number) % buckets;
} 

unsigned int handle_ip_count(void *ip)
{
	// 当一个客户登录的时候，要在s_ip_count_hash更新这个表中的对应表项,
	// 即该ip对应的连接数要加1，如果这个表项还不存在，要在表中添加一条记录，
	// 并且将ip对应的连接数置1。

	unsigned int count;
	unsigned int *p_count=(unsigned int*)hash_lookup_entry(s_ip_count_hash,ip,sizeof(unsigned int));
	if (p_count==NULL)
	{
		count=1;
		hash_add_entry(s_ip_count_hash,ip,sizeof(unsigned int),&count,sizeof(unsigned int));
	}
	else
	{
		count=*p_count;
		count++;
		*p_count=count;
	}
	return count;
}

void drop_ip_count(void *ip)
{
	// 得到了ip进而我们就可以在s_ip_count_hash表中找到对应的连接数，进而进行减1操作。
	unsigned int count;
	unsigned int *p_count=(unsigned int*)hash_lookup_entry(s_ip_count_hash,ip,sizeof(unsigned int));
	if (p_count==NULL)
		return ;
	count = *p_count;
	if(count<=0)
		return ;
	count--;
	*p_count=count;
	if(count==0)
		hash_free_entry(s_ip_count_hash,ip,sizeof(unsigned int));
}
