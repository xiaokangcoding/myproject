#include "sysutil.h"
#include "common.h"

int tcp_client(const unsigned short port)
{
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock==-1)
		ERR_EXIT("tcp_client");
	if(port>0)
	{
		struct sockaddr_in localaddr;
		memset(&localaddr,0,sizeof(localaddr));
		localaddr.sin_family=AF_INET;
		localaddr.sin_port=htons(port);
		//char ip[16]={0};
		//getlocalip(ip);
		localaddr.sin_addr.s_addr=inet_addr("10.1.2.242");
		int on=1;
		if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(int))<0)
			ERR_EXIT("setsockopt");
		socklen_t addrlen=sizeof(struct sockaddr);
		if(bind(sock,(struct sockaddr*)&localaddr,addrlen)<0)
			ERR_EXIT("bind");
	}
	return sock;
}
/**
 * tcp_server - 启动tcp服务器
 * @host: 服务器IP地址或服务器主机名
 * @port: 服务器端口
 * 成功返回监听套接字
 */
int tcp_server(const char *host,unsigned short port)
{
	int listenfd;
	if( (listenfd=socket(PF_INET,SOCK_STREAM,0))<0)
		ERR_EXIT("tcp_server");
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	
	if(host!=NULL)
	{
		if(inet_aton(host,&servaddr.sin_addr)==0)//说明不是一个有效的ip地址  是服务器的主机名
		{
			//根据服务器的主机名得到服务器的ip地址
			struct hostent *hp;
			hp=gethostbyname(host); //通过主机名得到该主机的ip地址
			if(hp==NULL)
				ERR_EXIT("gethostbyname");
			servaddr.sin_addr=*(struct in_addr*)hp->h_addr;//设置服务器的ip地址
		}
	}
	else
		//获取本机任意的一个ip地址
		//servaddr.sin_addr.s_addr=htonl(INADDR_ANY);//INADDR_ANY 0     
		servaddr.sin_addr.s_addr=inet_addr("10.1.2.242");
		// htonl: 将主机的无符号长整形数转换成网络字节顺序
	//如果port=0  服务器将动态选择一个临时端口号
	servaddr.sin_port=htons(port);//将主机的无符号短整形数转换成网络字节顺序

	int on=1;
	if( (setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(int)) )<0)
		ERR_EXIT("setsockopt");

	printf("%s\n",inet_ntoa(servaddr.sin_addr));

	if( (bind(listenfd,(struct sockaddr*)&servaddr,sizeof(struct sockaddr)) )<0)
		ERR_EXIT("bind111");
	if(listen(listenfd,SOMAXCONN)<0)
		ERR_EXIT("listen");
	return listenfd;
}
int getlocalip(char *ip) //获取本地ip地址的函数
{
	char host[100] = {0};
	if (gethostname(host, sizeof(host)) < 0) //获取本机的主机名
		return -1;
	struct hostent *hp;
	if ((hp = gethostbyname(host)) == NULL)//用主机名获取ip地址
		return -1;

	strcpy(ip, inet_ntoa(*(struct in_addr*)hp->h_addr));//  存在ip中
	//char *inet_ntoa(struct in_addr in)
	return 0;
}
/**
 * activate_noblock - 设置I/O为非阻塞模式
 * @fd: 文件描符符
 */
void activate_nonblock(int fd)
{
	int ret;
	int flags = fcntl(fd, F_GETFL);
	if (flags == -1)
		ERR_EXIT("fcntl");

	flags |= O_NONBLOCK;
	ret = fcntl(fd, F_SETFL, flags);
	if (ret == -1)
		ERR_EXIT("fcntl");
}

/**
 * deactivate_nonblock - 设置I/O为阻塞模式
 * @fd: 文件描符符
 */
void deactivate_nonblock(int fd)
{
	int ret;
	int flags = fcntl(fd, F_GETFL);
	if (flags == -1)
		ERR_EXIT("fcntl");

	flags &= ~O_NONBLOCK;
	ret = fcntl(fd, F_SETFL, flags);
	if (ret == -1)
		ERR_EXIT("fcntl");
}

/**
 * readn - 读取固定字节数
 * @fd: 文件描述符
 * @buf: 接收缓冲区
 * @count: 要读取的字节数
 * 成功返回count，失败返回-1，读到EOF返回<count
 */

ssize_t readn(int fd, void *buf, size_t count)
{
	size_t nleft = count;
	ssize_t nread;
	char *bufp = (char*)buf;

	while (nleft > 0) {
		if ((nread = read(fd, bufp, nleft)) < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		else if (nread == 0)
			return count - nleft;

		bufp += nread;
		nleft -= nread;
	}

	return count;
}

/**
 * readline - 按行读取数据
 * @sockfd: 套接字
 * @buf: 接收缓冲区
 * @maxline: 每行最大长度
 * 成功返回>=0，失败返回-1
 */

ssize_t readline(int fd,void *vptr,size_t maxlen)
{
	ssize_t n,rc;
	char c,*ptr;
	ptr=vptr;
	for(n=1;n<maxlen;n++)
	{
		again:
			if( (rc=read(fd,&c,1))==1 ){
				*ptr++=c;
				if(c=='\n')
					break;
		}else if(rc==0){
			*ptr=0;
			return n-1;
		}else{
			if(errno==EINTR)
				goto again;
			return -1;
		}
	}
	*ptr=0;
	return n;
}
/**
 * writen - 发送固定字节数
 * @fd: 文件描述符
 * @buf: 发送缓冲区
 * @count: 要读取的字节数
 * 成功返回count，失败返回-1
 */

ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t nleft = n;
	ssize_t nwritten;
	char *ptr = (char*)vptr;

	while (nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft)) <=0) {
			if(nwritten<0 && errno==EINTR)
				nwritten = 0;    //call write() again
			else
				return -1;//error
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return n;
}
/**
 * accept_timeout - 带超时的accept
 * @fd: 套接字
 * @addr: 输出参数，返回对方地址
 * @wait_seconds: 等待超时秒数，如果为0表示正常模式
 * 成功（未超时）返回已连接套接字，超时返回-1并且errno = ETIMEDOUT
 */
int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
	int ret;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	if (wait_seconds > 0) {
		fd_set accept_fdset;
		struct timeval timeout;
		FD_ZERO(&accept_fdset);
		FD_SET(fd, &accept_fdset);
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do {
			ret = select(fd + 1, &accept_fdset, NULL, NULL, &timeout);
		} while (ret < 0 && errno == EINTR);
        if (ret == -1) {
			return -1;
        }
		else if (ret == 0) {
			errno = ETIMEDOUT;
			return -1;
		}
	}

	if (addr != NULL)
		ret = accept(fd, (struct sockaddr*)addr, &addrlen);
	else
		ret = accept(fd, NULL, NULL);

	return ret;
}

/**
 * connect_timeout - connect
 * @fd: 套接字
 * @addr: 要连接的对方地址
 * @wait_seconds: 等待超时秒数，如果为0表示正常模式
 * 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT
 */
int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
	int ret;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	if (wait_seconds > 0)
		activate_nonblock(fd);

	ret = connect(fd, (struct sockaddr*)addr, addrlen);
	//printf("%d\n",ret); -1
	if (ret < 0 && errno == EINPROGRESS) {
		fd_set connect_fdset;
		struct timeval timeout;
		FD_ZERO(&connect_fdset);
		FD_SET(fd, &connect_fdset);
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do {
			ret = select(fd + 1, NULL, &connect_fdset, NULL, &timeout);
		} while (ret < 0 && errno == EINTR);
		if (ret == 0) {
			ret = -1;
			errno = ETIMEDOUT;
		}
		else if (ret < 0)
			return -1;
		else if (ret == 1) {
			//printf("BBBBB\n");
			/* ret返回为1，可能有两种情况，一种是连接建立成功，一种是套接字产生错误，*/
			/* 此时错误信息不会保存至errno变量中，因此，需要调用getsockopt来获取。 */
			int err;
			socklen_t socklen = sizeof(err);
			int sockoptret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &socklen);
			if (sockoptret == -1) {
				return -1;
			}
			if (err == 0) {
			//	printf("DDDDDDD\n");
				ret = 0;
			} else {
				//printf("CCCCCC\n");
				errno = err;
				ret = -1;
			}
		}
	}
	if (wait_seconds > 0) {
		deactivate_nonblock(fd);
	}
	return ret;
}


//发送文件描述符
void send_fd(int sock_fd, int fd)  
{
	int ret;
	struct msghdr msg;
	struct cmsghdr *p_cmsg;
	struct iovec vec;
	char cmsgbuf[CMSG_SPACE(sizeof(fd))];
	int *p_fds;
	char sendchar = 0;
	msg.msg_control = cmsgbuf;
	msg.msg_controllen = sizeof(cmsgbuf);
	p_cmsg = CMSG_FIRSTHDR(&msg);
	p_cmsg->cmsg_level = SOL_SOCKET;
	p_cmsg->cmsg_type = SCM_RIGHTS;
	p_cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
	p_fds = (int*)CMSG_DATA(p_cmsg);
	*p_fds = fd;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &vec;
	msg.msg_iovlen = 1;
	msg.msg_flags = 0;

	vec.iov_base = &sendchar;
	vec.iov_len = sizeof(sendchar);
	ret = sendmsg(sock_fd, &msg, 0);
	if (ret != 1)
		ERR_EXIT("sendmsg");
}
//接收文件描述符
int recv_fd(const int sock_fd)
{
	int ret;
	struct msghdr msg;
	char recvchar;
	struct iovec vec;
	int recv_fd;
	char cmsgbuf[CMSG_SPACE(sizeof(recv_fd))];
	struct cmsghdr *p_cmsg;
	int *p_fd;
	vec.iov_base = &recvchar;
	vec.iov_len = sizeof(recvchar);
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &vec;
	msg.msg_iovlen = 1;
	msg.msg_control = cmsgbuf;
	msg.msg_controllen = sizeof(cmsgbuf);
	msg.msg_flags = 0;

	p_fd = (int*)CMSG_DATA(CMSG_FIRSTHDR(&msg));
	*p_fd = -1;  
	ret = recvmsg(sock_fd, &msg, 0);
	if (ret != 1)
		ERR_EXIT("recvmsg");

	p_cmsg = CMSG_FIRSTHDR(&msg);
	if (p_cmsg == NULL)
		ERR_EXIT("no passed fd");


	p_fd = (int*)CMSG_DATA(p_cmsg);
	recv_fd = *p_fd;
	if (recv_fd == -1)
		ERR_EXIT("no passed fd");

	return recv_fd;
}
const char* statbuf_get_perms(struct stat *sbuf)//获取文件权限
{
	static char perms[] = "----------";
	perms[0] = '?';//不确定文件的类型
	mode_t mode = sbuf->st_mode;
	switch (mode & S_IFMT) {//文件的类型 
	case S_IFREG:
		perms[0] = '-';//普通文件
		break;
	case S_IFDIR:
		perms[0] = 'd';//目录文件
		break;
	case S_IFLNK:
		perms[0] = 'l';//符号连接文件
		break;
	case S_IFIFO:
		perms[0] = 'p';//管道文件
		break;
	case S_IFSOCK:
		perms[0] = 's';//套接字文件
		break;
	case S_IFCHR:
		perms[0] = 'c';//字符设备文件
		break;
	case S_IFBLK:
		perms[0] = 'b';//块设备文件
		break;
	}
//拥有者的权限
	if (mode & S_IRUSR) {
		perms[1] = 'r';//有读权限
	}
	if (mode & S_IWUSR) {
		perms[2] = 'w';//有写权限
	}
	if (mode & S_IXUSR) {//有可执行权限
		perms[3] = 'x';
	}
	//组用户的权限
	if (mode & S_IRGRP) {
		perms[4] = 'r';
	}
	if (mode & S_IWGRP) {
		perms[5] = 'w';
	}
	if (mode & S_IXGRP) {
		perms[6] = 'x';
	}
	//其他用户的权限
	if (mode & S_IROTH) {
		perms[7] = 'r';
	}
	if (mode & S_IWOTH) {
		perms[8] = 'w';
	}
	if (mode & S_IXOTH) {
		perms[9] = 'x';
	}
	//特殊权限位
	if (mode & S_ISUID) {
		perms[3] = (perms[3] == 'x') ? 's' : 'S';
	}
	if (mode & S_ISGID) {
		perms[6] = (perms[6] == 'x') ? 's' : 'S';
	}
	if (mode & S_ISVTX) {
		perms[9] = (perms[9] == 'x') ? 't' : 'T';
	}

	return perms;
}

const char* statbuf_get_date(struct stat *sbuf)
{
	 static char datebuf[64]={0};
	 //获取系统当前时间 int gettimeofday(struct timeval *tv, struct timezone *tz);
	 const char *p_data_format="%b %e %H:%M";
	 struct timeval tv;
	 gettimeofday(&tv,NULL);
	 time_t local_time=tv.tv_sec;
	 if(sbuf->st_mtime>local_time || (local_time - sbuf->st_mtime)>182*24*60*60)//??????????
	 p_data_format="%b %e   %Y";
	
	 struct tm *p_tm=localtime(&local_time);
	 //size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);
	 strftime(datebuf,sizeof(datebuf),p_data_format,p_tm);
	 return datebuf;
}
static int lock_internal(int fd,int lock_type)//static 表示该函数只对该文件有效
{
	struct flock the_lock;//锁的结构体
	memset(&the_lock,0,sizeof(the_lock));
	the_lock.l_type=lock_type;//锁的类型

	the_lock.l_whence=SEEK_SET;//加锁的位置  从文件的头部
	the_lock.l_start=0;        //加锁的偏移量

	the_lock.l_len=0;//0:表示将整个文件进行加锁
	int ret;
	do
	{
		ret=fcntl(fd,F_SETLKW,&the_lock);//F_SETLKW 加锁失败 就阻塞 直到其他进程释放该锁
	}
	while (ret<0 && errno==EINTR); //EINTR:表示被信号中断
	//出了循环  ret==0 表示成功  ret<0,errno!=EINTR
	return ret;
}
int lock_file_read(int fd)
{
	return lock_internal(fd,F_RDLCK);
}

int lock_file_write(int fd)
{
	return lock_internal(fd,F_WRLCK);
}
int unlock_fd(int fd)//解锁函数
{
	struct flock the_lock;//锁的结构体
	memset(&the_lock,0,sizeof(the_lock));
	the_lock.l_type=F_UNLCK;//锁的类型

	the_lock.l_whence=SEEK_SET;//加锁的位置  从文件的头部
	the_lock.l_start=0;        //加锁的偏移量

	the_lock.l_len=0;//0:表示将整个文件进行加锁
	int ret=fcntl(fd,F_SETLK,&the_lock);//F_SETLK 解锁失败 直接返回
	return ret;
}

//获取当前时间的函数
static struct timeval s_curr_time;

long get_time_sec(void)
{
	if (gettimeofday(&s_curr_time,NULL)<0)
		ERR_EXIT("gettimeofday");
	return s_curr_time.tv_sec;
}
long get_time_usec(void)
{
	return s_curr_time.tv_usec;
}

void nano_sleep(double seconds)
{
	time_t secs = (time_t)seconds;					// 整数部分
	double fractional = seconds - (double)secs;		// 小数部分

	struct timespec ts;
	ts.tv_sec = secs;
	ts.tv_nsec = (long)(fractional * (double)1000000000);
	
	int ret;
	do 
	{
		ret = nanosleep(&ts, &ts);    //睡眠一定的时间
	}
	while (ret == -1 && errno == EINTR);
}

void activate_oobinline(int fd)
{
	int oob_inline=1;// 1 代表开启
	int ret;
	ret=setsockopt(fd,SOL_SOCKET,SO_OOBINLINE,&oob_inline,sizeof(oob_inline));
	if(ret==-1)
		ERR_EXIT("setsockopt");
}
void activate_sigurg(int fd)
{
	int ret;
	ret=fcntl(fd,F_SETOWN,getpid());
	if(ret==-1)
		ERR_EXIT("fcntl");
}
