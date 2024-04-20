  #ifndef _SYS_UTIL_H_
#define _SYS_UTIL_H_
#include "common.h"

int getlocalip(char *ip);

void activate_nonblock(int fd);  
void deactivate_nonblock(int fd);

//host ����������ip��ַ
int tcp_server(const char *host,unsigned short port);

int tcp_client(const unsigned short port);

//int read_timeout(int fd, unsigned int wait_seconds);
//int write_timeout(int fd, unsigned int wait_seconds);
int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);
int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);


ssize_t writen(int fd, const void *vptr, size_t n);
ssize_t readn(int fd, void *buf, size_t count);
ssize_t readline(int fd,void *vptr,size_t maxlen);
//ssize_t recv_peek(int sockfd, void *buf, size_t len);

const char* statbuf_get_perms(struct stat *sbuf);
const char* statbuf_get_date(struct stat *sbuf);
void send_fd(int sock_fd, int fd);
int recv_fd(const int sock_fd);

int lock_file_read(int fd);
int lock_file_write(int fd);
int unlock_fd(int fd);
//��ȡ��ǰʱ��ĺ���
long get_time_sec(void);
long get_time_usec(void);

void nano_sleep(double seconds);
//�����׽���fd���մ������ݵĹ���
void activate_oobinline(int fd);
//���ļ�������fd���д�������ʱ ������SIGURG�ź�
void activate_sigurg(int fd);
#endif