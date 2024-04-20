#ifndef _PRIV_SOCK_H_
#define _PRIV_SOCK_H_
#include"session.h"

// 内部进程自定义协议
// 用于FTP服务进程与nobody进程进行通信

// FTP服务进程向nobody进程请求的命令
//主动模式相关
#define PRIV_SOCK_GET_DATA_SOCK     1	//向nobody进程获取主动模式数据套接字
//被动模式相关
#define PRIV_SOCK_PASV_ACTIVE       2
#define PRIV_SOCK_PASV_LISTEN       3
#define PRIV_SOCK_PASV_ACCEPT       4

// nobody进程对FTP服务进程的应答

#define PRIV_SOCK_RESULT_OK         1
#define PRIV_SOCK_RESULT_BAD        2



void priv_sock_init(session_t *sess); //初始化内部进程间通信通道
void priv_sock_close(session_t *sess);//关闭内部进程间通信通道
void priv_sock_set_parent_context(session_t *sess);//设置父进程环境
void priv_sock_set_child_context(session_t *sess);//设置子进程环境
////////////////////////////////////////////////////////
void priv_sock_send_cmd(int fd, char cmd);//ftp服务进程向nobody进程发送命令 传输一个字节
char priv_sock_get_cmd(int fd);
void priv_sock_send_result(int fd, char res);    
char priv_sock_get_result(int fd);
////////////////////////////////////////////////////////
void priv_sock_send_int(int fd, int the_int);
int priv_sock_get_int(int fd);
void priv_sock_send_buf(int fd, const char *buf, unsigned int len);
void priv_sock_recv_buf(int fd, char *buf, unsigned int len);
void priv_sock_send_fd(int sock_fd, int fd);
int priv_sock_recv_fd(int sock_fd);

///////////////////////////////////////////////////////////
#endif