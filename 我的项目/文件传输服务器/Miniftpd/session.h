#ifndef _SESSION_H_
#define _SESSION_H_

#include "common.h"

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
void begin_session(session_t *sess);
#endif