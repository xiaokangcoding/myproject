#include "ftpproto.h"
#include "sysutil.h"
#include "str.h"
#include "ftpcodes.h"
#include "tunable.h"
#include "privsock.h"
#include "session.h"

void start_cmdio_alarm();
void start_data_alarm();

void handle_sigalrm(int sig);
void handle_alarm_timeout(int sig);
void handle_sigurg(int sig);

//void check_abor(session_t *sess);

int list_common(session_t *sess,int detail);  //将当前目录列出来
void upload_common(session_t *sess,int is_append);//上传
void limit_rate(session_t *sess, int bytes_transfered, int is_upload);//限速


int get_transferfd(session_t *sess);//返回创建的数据连接的套接字
int port_active(session_t *sess);
int pasv_active(session_t *sess);
int get_port_fd(session_t *sess);
int get_pasv_fd(session_t *sess);

void ftp_reply(session_t *sess,int status,const char *text);
void ftp_lreply(session_t *sess,int status,const char *text);
static void do_user(session_t *sess);//验证用户
static void do_pass(session_t *sess);//验证密码
static void do_syst(session_t *sess); //响应客户端当前系统的类型
static void do_feat(session_t *sess);//请求服务器端的特征
static void do_pwd(session_t *sess); //显示当前的目录
static void do_type(session_t *sess);//传输模式是否是ASCII模式
static void do_port(session_t *sess);
static void do_list(session_t *sess);
static void do_pasv(session_t *sess);
static void do_nlst(session_t *sess);
static void do_cwd(session_t *sess);
static void do_cdup(session_t *sess);
static void do_mkd(session_t *sess);//建立目录(文件夹)
static void do_rmd(session_t *sess);//删除目录(文件夹)
static void do_dele(session_t *sess);//删除文件
static void do_rest(session_t *sess);
static void do_size(session_t *sess);
//以下两个函数是进行文件的重命名的  必须先发rnfr 再发rnto
static void do_rnfr(session_t *sess);
static void do_rnto(session_t *sess);

static void do_retr(session_t *sess);//下载文件   断点续传
static void do_stor(session_t *sess);//上传文件   断点续传

static void do_noop(session_t *sess);
static void do_quit(session_t *sess);
static void do_abor(session_t *sess);
/*static void do_stru(session_t *sess);
//static void do_mode(session_t *sess);
static void do_appe(session_t *sess);

static void do_site(session_t *sess);
static void do_stat(session_t *sess);
static void do_help(session_t *sess);
//FTP命令与命令处理函数对应表
static void do_site_chmod(session_t *sess, char *chmod_arg);
static void do_site_umask(session_t *sess, char *umask_arg);*/
typedef struct ftpcmd {
	const char *cmd; //命令字符串
	void (*cmd_handler)(session_t *sess);
} ftpcmd_t;

static ftpcmd_t ctrl_cmds[] = {
	/* 访问控制命令 */
	{"USER",	do_user	},
	{"PASS",	do_pass	},
	{"SYST",	do_syst },
	{"FEAT",	do_feat },//暂且不实现do_feat()命令
	{"PWD" ,	do_pwd	},
	{"TYPE",	do_type	},
	{"PORT",	do_port },
	{"LIST",	do_list },
	{"PASV",	do_pasv },
	{"NLST",	do_nlst },
	{"CWD" ,        do_cwd  },
	{"CDUP",	do_cdup },
	{"MKD" ,	do_mkd	},
	{"RMD" ,	do_rmd	},
	{"DELE",	do_dele },
	{"REST",	do_rest },
	{"SIZE",	do_size },
	{"RNFR",	do_rnfr },
	{"RNTO",	do_rnto },
	{"RETR",	do_retr },
	{"RNTO",	do_rnto	},
	{"STOR",	do_stor },
	{"NOOP",	do_noop	},
	{"QUIT",	do_quit	}
};

//从客户端一行一行地接收数据
/*
客户端发送过来的命令顺序是不确定的
*/
session_t *p_sess;
/*
void check_abor(session_t *sess)
{
	if (sess->abor_received)
	{
		sess->abor_received=0;
		ftp_reply(p_sess, FTP_ABOROK, "ABOR successful.");
	}
}
*/
void handle_alarm_timeout(int sig)
{
	shutdown(p_sess->ctrl_fd,SHUT_RD);
	ftp_reply(p_sess,FTP_IDLE_TIMEOUT,"Timeout.");
	shutdown(p_sess->ctrl_fd,SHUT_WR);
	exit(EXIT_FAILURE);
}

void start_cmdio_alarm()
{
	if(tunable_idle_session_timeout>0)
	{
		//安装信号
		signal(SIGALRM,handle_alarm_timeout);
		//启动闹钟
		alarm(tunable_idle_session_timeout);
	}
}

void handle_sigalrm(int sig)
{

	if (!p_sess->data_process )
	{
		ftp_reply(p_sess, FTP_DATA_TIMEOUT, "Data timeout. Reconnect. Sorry.");
		exit(EXIT_FAILURE);
	}
	printf("11111111111111111111111\n");
	// 否则，当前处于数据传输的状态收到了超时信号
	p_sess->data_process = 0;
	printf("%d\n",p_sess->data_process);
	start_data_alarm();
}

//一旦产生了SIGURG信号，意味着发送了一个带外数据
void handle_sigurg(int sig)
{
	if (p_sess->data_fd==-1)
		return ;
	//处于数据连接状态
	//char cmdline[MAX_COMMAND_LINE]={0};
	//按行接收
	int ret = readline(p_sess->ctrl_fd,p_sess->cmdline,MAX_COMMAND_LINE);
	if(ret<=0)
		ERR_EXIT("readline");
	//解析命令
//	首先去除\r\n
	
	str_trim_crlf(p_sess->cmdline);
	if( strcmp(p_sess->cmdline,"\377\364\377\362ABOR")==0
		|| strcmp(p_sess->cmdline,"ABOR")==0 )
	{
		p_sess->abor_received=1; //该变量表明是否收到abor命令
		shutdown(p_sess->data_fd,SHUT_RDWR);//断开数据连接通道
	}
	else
		ftp_reply(p_sess, FTP_BADCMD, "Unknown command.");

}

void start_data_alarm()
{
	if (tunable_data_connection_timeout>0)
	{
		//安装信号
		signal(SIGALRM,handle_sigalrm);
		//启动闹钟
		alarm(tunable_data_connection_timeout);
	}
	else if(tunable_idle_session_timeout>0)
	{
		//关闭先前的闹钟
		alarm(0);
	}
}
void handle_child(session_t *sess)
{
	ftp_reply(sess,FTP_GREET,"(miniftpd 0.1)");
	while(1)
	{
		memset(sess->cmdline,0,MAX_COMMAND_LINE);
		memset(sess->cmd,0,MAX_COMMAND);
		memset(sess->arg,0,MAX_ARG);

		start_cmdio_alarm();
		int ret=readline(sess->ctrl_fd,sess->cmdline,MAX_COMMAND_LINE);
		if(ret==-1)
			ERR_EXIT("readline");//关闭ftp服务进程   此时也要关闭nobody进程
		else if(ret==0)//表示客户端断开了连接
			exit(EXIT_SUCCESS);//关闭ftp服务进程     此时也要关闭nobody进程
		
		str_trim_crlf(sess->cmdline);//去除\r\n
		printf("cmdline=[%s]\n",sess->cmdline);
		//解析命令与参数-----FTP命令
		str_split(sess->cmdline,sess->cmd,sess->arg,' ');
		printf("cmd=[%s] arg=[%s]\n",sess->cmd,sess->arg);
		//将命令转换成大写
		str_upper(sess->cmd);
		//处理FTP命令---可能需要nobody进程的协助 这时候可能需要向nobody进程发送内部命令

		/*if(strcmp("USER",sess->cmd)==0)
		{
			do_user(sess);
		}
		else if(strcmp("PASS",sess->cmd)==0)
		{
			do_pass(sess);
		}*/
		//FTP命令的映射
		int i;
		int size=sizeof(ctrl_cmds)/sizeof(ctrl_cmds[0]);
		for(i=0;i<size;i++)
		{
			if(strcmp(ctrl_cmds[i].cmd,sess->cmd)==0)//说明命令匹配
			{
				if(ctrl_cmds[i].cmd_handler!=NULL)
					ctrl_cmds[i].cmd_handler(sess);
				else	//说明函数未实现
					ftp_reply(sess,FTP_COMMANDNOTIMPL,"UnImplement Command.");
				break;
			}
		}
		if(i==size)
		{
			//说明未找到命令
			ftp_reply(sess,FTP_BADCMD,"Unknown Command.");
		}
	}
}

//验证用户
static void do_user(session_t *sess)
{
	//USER jjl
	struct passwd *pw=getpwnam(sess->arg);
	if(pw==NULL)//很可能是不存在的用户
	{
		ftp_reply(sess,FTP_LOGINERR,"Login incorrect.");//530
		return ;
	}
	sess->uid=pw->pw_uid;//保存该用户的id
	ftp_reply(sess,FTP_GIVEPWORD,"Please specify the password.");//331
}

//验证密码
static void do_pass(session_t *sess)
{
	//PASS 123456(明文) 实际的密码是保存在影子文件中
	struct passwd *pw=getpwuid(sess->uid); //根据用户id得到密码结构体《====》获取密码文件信息
	
	if(pw==NULL)//很可能是不存在的用户
	{
		ftp_reply(sess,FTP_LOGINERR,"Login incorrect."); //530
		return ;
	}
	//printf("username=%s\n",pw->pw_name);
	struct spwd *sp=getspnam(pw->pw_name);//根据用户名 获取影子文件信息
	if(sp==NULL)
	{
		ftp_reply(sess,FTP_LOGINERR,"Login incorrect."); //530
		return ;
	}
	//首先将明文密码(123456)
	char *encrypt_pass=crypt(sess->arg,sp->sp_pwdp);//encrypt_pass：获得的已加密的密码
	//进行密码的验证
	if(strcmp(encrypt_pass,sp->sp_pwdp)!=0)//密码验证失败
	{
		ftp_reply(sess,FTP_LOGINERR,"Login incorrect."); //530
		return ;
	}

	signal(SIGURG,handle_sigurg);
	activate_sigurg(sess->ctrl_fd);

	umask(tunable_local_umask);
	//密码验证成功的时候  再设置下面的
	setgid(pw->pw_gid);//更改为实际用户的组id
	setuid(pw->pw_uid);//更改为实际用户的用户id
	chdir(pw->pw_dir);//更改为实际用户的家目录
	ftp_reply(sess,FTP_LOGINOK,"Login successfully."); //230
}
static void do_syst(session_t *sess)
{ 
	ftp_reply(sess,FTP_SYSTOK,"UNIX Type: L8");
}

void ftp_reply(session_t *sess,int status,const char *text)
{
	char buf[1024]={0};
	sprintf(buf,"%d %s\r\n",status,text);
	writen(sess->ctrl_fd,buf,strlen(buf));////////////+1  客户端就不会发送PASS命令
}


static void do_feat(session_t *sess)
{
	ftp_lreply(sess,FTP_FEAT,"Features:");
	writen(sess->ctrl_fd,"EPRT\r\n",strlen("EPRT\r\n"));
	writen(sess->ctrl_fd,"EPSV\r\n",strlen("EPSV\r\n"));
	writen(sess->ctrl_fd,"MDTM\r\n",strlen("MDTM\r\n"));
	writen(sess->ctrl_fd,"PASV\r\n",strlen("PASV\r\n"));
	writen(sess->ctrl_fd,"REST STREAM\r\n",strlen("REST STREAM\r\n"));
	//REST STREAM:说明该服务器支持断点续传  发生此消息之后 ftp客户端将不会发送REST命令
	writen(sess->ctrl_fd,"SIZE\r\n",strlen("SIZE\r\n"));
	writen(sess->ctrl_fd,"TVFS\r\n",strlen("TVFS\r\n"));
	writen(sess->ctrl_fd,"UTF8\r\n",strlen("UTF8\r\n"));
	ftp_reply(sess,FTP_FEAT,"End");
}
static void do_pwd(session_t *sess)
{
	char text[1024]={0};
	char dir[1024+1]={0};
	//获取当前目录
	getcwd(dir,1024);
	//printf("%s\n",dir);
	sprintf(text,"\"%s\"",dir);
	ftp_reply(sess,FTP_PWDOK,text);
}
static void do_type(session_t *sess)
{
	if(strcmp(sess->arg,"A")==0)
	{
		sess->is_ascii=1;
		ftp_reply(sess,FTP_TYPEOK,"Switching to ASCII mode.");
	}
	else if(strcmp(sess->arg,"I")==0)
	{
		sess->is_ascii=0;
		ftp_reply(sess,FTP_TYPEOK,"Switching to Binary mode.");
	}
	else
		ftp_reply(sess,FTP_BADCMD,"Unrecognised TYPE command."); 
}
void ftp_lreply(session_t *sess,int status,const char *text)
{
	char buf[1024]={0};
	sprintf(buf,"%d-%s\r\n",status,text);
	writen(sess->ctrl_fd,buf,strlen(buf));////////////+1  客户端就不会发送PASS命令
}

int list_common(session_t *sess,int detail)
{
	DIR *dir=opendir(".");//打开当前目录
	if(dir==NULL)
		return 0;
	//遍历当前目录中的文件 readdir()遍历
	struct dirent *dt;
	struct stat sbuf;
	char buf[1024]={0};
	while( (dt=readdir(dir))!=NULL)
	{
		if(lstat(dt->d_name,&sbuf)<0)//获取文件的状态  获取一个文件保存在另一个参数中
			continue;//如果获取文件失败 则continue  访问下一个文件
		if(dt->d_name[0]=='.')
			continue;
		/*lstat()  is  identical to stat(), except that if path is a symbolic link, then the link itself is stat-ed,
		 not the file that it refers to.*/
		 if(detail) //详细清单
		 {
			 const char *perms=statbuf_get_perms(&sbuf);
			
			 int off=0;
			 off += sprintf(buf,"%s ",perms);
			 off += sprintf(buf+off," %3d %-8d %-8d ",(int)sbuf.st_nlink,sbuf.st_uid,sbuf.st_gid);
			 off += sprintf(buf+off," %8lu  ",(unsigned long)sbuf.st_size);
																//当前系统的时区(NULL)
			 const char *datebuf=statbuf_get_date(&sbuf);
			 off += sprintf(buf+off,"%s ",datebuf);
			 if(S_ISLNK(sbuf.st_mode))
			 {
				 char temp[64]={0};
				 readlink(dt->d_name,temp,sizeof(temp));//获取链接文件所指文件 将该文件名保存在temp中
				 sprintf(buf+off,"%s -> %s\r\n",dt->d_name,temp);
			 }
			 else
				sprintf(buf+off,"%s\r\n",dt->d_name);
		 }
		 else
			 sprintf(buf,"%s\r\n",dt->d_name);

		 writen(sess->data_fd,buf,strlen(buf)); //发送给ftp客户端
		// printf("%s",buf);//打印至标准输出
	}
	//关闭目录
	closedir(dir);
	return 1;
}

//睡眠时间=(当前传输速度/最大传输速度-1)*当前传输时间
//限速实现
void limit_rate(session_t *sess, int bytes_transfered, int is_upload) //bytes_transfered:当前传输的字节数
{
	sess->data_process=1;

	printf("%d\n",p_sess->data_process);
	//当传输一定量的数据 再次获取当前的时间
	long curr_sec = get_time_sec();
	long curr_usec = get_time_usec();

	double elapsed;//当前传输时间
	elapsed = (double)(curr_sec - sess->bw_transfer_start_sec);
	elapsed += (double)(curr_usec - sess->bw_transfer_start_usec) / (double)1000000;
	if (elapsed <= (double)0) 
	{
		elapsed = (double)0.01;
	}
	
	// 计算当前传输速度
	unsigned int bw_rate = (unsigned int)((double)bytes_transfered / elapsed);
	double rate_ratio;
	if (is_upload) 
	{
		if (bw_rate <= sess->bw_upload_rate_max) 
		{
			// 不需要限速
			sess->bw_transfer_start_sec = curr_sec;
			sess->bw_transfer_start_usec = curr_usec;
			return;
		}
		rate_ratio = bw_rate / sess->bw_upload_rate_max;
	} 
	else 
	{
		if (bw_rate <= sess->bw_download_rate_max) 
		{
			// 不需要限速
			sess->bw_transfer_start_sec = curr_sec;
			sess->bw_transfer_start_usec = curr_usec;
			return;
		}
		rate_ratio = bw_rate / sess->bw_download_rate_max;
	}

	// 睡眠时间 = (当前传输速度 / 最大传输速度 – 1) * 当前传输时间;
	double pause_time;	// 睡眠时间
	pause_time = (rate_ratio - (double)1) * elapsed;

	nano_sleep(pause_time);

	sess->bw_transfer_start_sec = get_time_sec();
	sess->bw_transfer_start_usec = get_time_usec();
}

void upload_common(session_t *sess,int is_append)
{
	//上传文件   断点续传   客户端首先会发送PASV or PORT命令
	//创建数据连接
	if(get_transferfd(sess)==0)//创建数据连接失败	
		return ;
	long long offset=sess->restart_pos;
	sess->restart_pos=0;

	//先打开要上传的文件          
	int fd=open(sess->arg,O_CREAT|O_WRONLY,0666);
	if(fd==-1)
	{
		ftp_reply(sess,FTP_UPLOADFAIL,"Could not create file.");
		return ;
	}
	//给要打开的文件加写锁    原因:在上传文件的时候 不允许别人读该文件 更不允许别人写这个文件
	int ret=lock_file_write(fd);
	if(ret==-1)
	{
		ftp_reply(sess,FTP_FILEFAIL,"Lock file failed.");
		return ;
	}

	//lseek() 函数将与文件描述符fd关联的打开文件的偏移量重新定位到参数偏移量
	if(is_append!=0&& offset==0 )  //STOR
	{
		ftruncate(fd,0);//将文件清零？？？？？？？？？？？？？
		lseek(fd,offset,SEEK_SET);//SEEK_SET :偏移量设置为偏移字节
	}
	else if(is_append!=0 && offset!=0) //REST ---	STOR
	{
		if (lseek(fd,offset,SEEK_SET)<0)
		{
			ftp_reply(sess,FTP_FILEFAIL,"dingwei weizhi failed!");
			return ;
		}
	}

	struct stat sbuf;
	if (fstat(fd,&sbuf)<0)
	{
		printf("get file failed!");
		return ;
	}
	//150
	char text[1024];
	sprintf(text, "Ok to send data.");
#if 0
	char text[1024];
	if(sess->is_ascii)
	{
		sprintf(text,"Opening ASCII mode data connection for %s (%lld)",
			sess->arg,(long long)sbuf.st_size);
	}
	else
	{
		sprintf(text,"Opening BINARY mode data connection for %s (%lld)",
			sess->arg,(long long)sbuf.st_size);
	}
#endif
	//150 : 开始传输文件
	ftp_reply(sess,FTP_DATACONN,text);//150

	//上传文件
	int flag=0;//标志位 表示对应的几种情况
	char buf[1024]={0};

	/*开始传输的时间*/
	sess->bw_transfer_start_sec=get_time_sec();//获取当前时间的秒数
	sess->bw_transfer_start_usec=get_time_usec();//获取当前时间的微妙数

	while (1)
	{
		ret=read(sess->data_fd,buf,sizeof(buf));//返回值是 当前读取的字节数
		if(ret==-1)
		{
			if(errno==EINTR)
				continue;
			else
			{
				flag=2;//表示错误
				break;
			}
		}
		else if(ret==0)//表示读取到了文件末尾  客户端断开了连接
		{
			flag=0;
			break;
		}

		limit_rate(sess,ret,1);
		if (sess->abor_received)
		{
			flag=1;
			break;
		}
	   //读取到了一定的数据  写到数据套接字里
		if (writen(fd,buf,ret) != ret)
		{
			flag=1;
			break;
		}
	}
	//关闭数据套接字
	close(sess->data_fd);
	sess->data_fd=-1;
	close(fd);
	if(flag==0)
	{
		//226
		ftp_reply(sess,FTP_TRANSFEROK,"Transfer complete.");
	}
	else if(flag==1)
	{
		//426
		ftp_reply(sess,FTP_BADSENDNET,"Failure writing to local file.");
	}
	else if(flag==2)
	{
		//451
       ftp_reply(sess,FTP_BADSENDFILE,"Failure reading from network stream .");
	}

	start_cmdio_alarm();//重新开启控制连接通道闹钟  原因是 之前的闹钟有可能关闭
}
//在创建数据连接通道之前  首先要协商使用PORT还是使用PASV模式
static void do_port(session_t *sess)
{
	//PORT 192,168,127,1,253,127
	unsigned int v[6];
	sscanf(sess->arg,"%u,%u,%u,%u,%u,%u",&v[2],&v[3],&v[4],&v[5],&v[0],&v[1]);
	sess->port_addr=(struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	memset(sess->port_addr,0,sizeof(struct sockaddr_in));
	sess->port_addr->sin_family=AF_INET;
	unsigned char *p=(unsigned char *)&(sess->port_addr->sin_port);
	p[0]=v[0];
	p[1]=v[1];
	//printf("%d\n",ntohs(sess->port_addr->sin_port));
	p=(unsigned char*)&(sess->port_addr->sin_addr);
	p[0]=v[2];
	p[1]=v[3];
	p[2]=v[4];
	p[3]=v[5];
	printf("%s\n",inet_ntoa(sess->port_addr->sin_addr));
	ftp_reply(sess,FTP_PORTOK,"command successful. Consider using PASV.");//200
}
int pasv_active(session_t *sess)
{
	
/*	if(sess->pasv_listenfd!=-1)
	{
		if(sess->port_addr!=NULL)
		{
			fprintf(stderr,"both port and pasv are active!\n");
			exit(EXIT_FAILURE);
		}
		return 1;
	}
	return 0;*/
	

	priv_sock_send_int(sess->child_fd,PRIV_SOCK_PASV_ACTIVE);
	int active=priv_sock_get_int(sess->child_fd);
	//printf("%d\n",active);
	if(active)
	{
		if(sess->port_addr!=NULL)
		{
			fprintf(stderr,"both port and pasv are active!\n");
			exit(EXIT_FAILURE);
		}
		return 1;
	}
	return 0;
}

static void do_pasv(session_t *sess)
{
 	// Entering Passive Mode (192,168,76,6,75,175).
	 char ip[16]; 
	// getlocalip(ip);//获取本地ip地址   // printf("%s\n",ip);
	
	 priv_sock_send_cmd(sess->child_fd,PRIV_SOCK_PASV_LISTEN);//向nobody进程获取pasv模式的监听端口
	 unsigned short port=priv_sock_get_int(sess->child_fd);
	
	 strcpy(ip,"10.1.2.242");//getlocalip(ip);
	 unsigned int v[4];//ip地址
	 sscanf(ip,"%u.%u.%u.%u",&v[0],&v[1],&v[2],&v[3]);
	 char text[1024]={0};
	 sprintf(text,"Entering Passive Mode (%u,%u,%u,%u,%u,%u).",v[0],v[1],v[2],v[3],port>>8,port & 0xFF);
	 ftp_reply(sess,FTP_PASVOK,text);
}
int port_active(session_t *sess)
{
	if(sess->port_addr!=NULL)  //两种模式不能同时处于激活状态
	{
		if(pasv_active(sess))
		{
			fprintf(stderr,"both port and pasv are active!\n");
			exit(EXIT_FAILURE);
		}
		return 1;
	}
	return 0;
}
int get_port_fd(session_t *sess)
{		
	/*
		FTP服务进程接收PORT h1,h2,h3,h4,p1,p2---------------
		解析 ip port----------------------------------------前2条已在do_port函数里面实现

		向nobody进程发送PRIV_SOCK_GET_DATA_SOCK命令	1
		向nobody进程发送一个整数port				4
		向nobody进程发送一个字符串 ip				不定长    
	*/
		unsigned short port=ntohs(sess->port_addr->sin_port);
		char *ip=inet_ntoa(sess->port_addr->sin_addr);
		priv_sock_send_cmd(sess->child_fd,PRIV_SOCK_GET_DATA_SOCK);
		priv_sock_send_int(sess->child_fd,(int)port);
		priv_sock_send_buf(sess->child_fd,ip,strlen(ip));
	
		char res = priv_sock_get_result(sess->child_fd);
	
		if(res==PRIV_SOCK_RESULT_BAD) //收到失败的应答-----则说明nobody进程创建数据连接通道失败
			return 0;//失败
		else if(res==PRIV_SOCK_RESULT_OK)
			sess->data_fd=priv_sock_recv_fd(sess->child_fd);  //保存成功的数据套接字
		return 1; //返回1 成功
}

int get_pasv_fd(session_t *sess)//获取被动模式的数据套接字
{
	priv_sock_send_cmd(sess->child_fd,PRIV_SOCK_PASV_ACCEPT);
	char res = priv_sock_get_result(sess->child_fd);
	if(res==PRIV_SOCK_RESULT_BAD) //收到失败的应答-----则说明nobody进程创建数据连接通道失败
			return 0;//失败
		else if(res==PRIV_SOCK_RESULT_OK)
			sess->data_fd=priv_sock_recv_fd(sess->child_fd);  //保存成功的数据套接字
	return 1; //返回1 成功
}
//数据连接通道的创建有两种模式: 主动模式  被动模式
int get_transferfd(session_t *sess)
{
	/*
	FTP服务进程接收PORT h1,h2,h3,h4,p1,p2
	解析 ip port
	向nobody进程发送PRIV_SOCK_GET_DATA_SOCK命令	1
	向nobody进程发送一个整数port				4
	向nobody进程发送一个字符串 ip				不定长    
	*/

	//检测是否收到PORT或PASV命令
	if(!port_active(sess) && !pasv_active(sess))	//|| !pasv_active(sess)
	{
		ftp_reply(sess,FTP_BADSENDCONN,"Use PORT or PASV first.");
		return 0;
	}
	int ret=1;  //将ret默认置为1
	if(port_active(sess))//如果是PORT模式
	{
	/*	socket
		bind 20 
		connect*/
	//	tcp_client(20);
/*		int fd=tcp_client(0);//sess->data_fd 数据连接套接字
		//printf("%s\n",inet_ntoa(sess->port_addr->sin_addr));  
		if(connect_timeout(fd, sess->port_addr, tunable_connect_timeout)<0)
		{
			close(fd);
			return 0;//返回假
		}
		//连接成功
		sess->data_fd=fd;*/
		//将以上代码替换为 让nobody进程协助ftp服务进程来创建数据连接通道
		//步骤如下:
		if(get_port_fd(sess)==0)   //获取nobody进程发送过来的数据连接套接字
			ret=0;  //返回数据套接字失败    ret置为0
	}
	if (sess->port_addr!=NULL)
	{
		free(sess->port_addr);
		sess->port_addr=NULL;
	}
	if(pasv_active(sess) )	//如果是被动模式的话
	{
		/*int fd=accept_timeout(sess->pasv_listenfd,NULL,tunable_accept_timeout);//NULL表示：不需要客户端的地址信息
		close(sess->pasv_listenfd);//不论accept_timeout创建是否成功  都要关闭监听套接字pasv_listenfd
		if(fd==-1)	
			return 0; 
		sess->data_fd=fd;*/
		if(get_pasv_fd(sess)==0)
			ret=0;  //返回数据套接字失败    ret置为0
	}
	if (ret)
	{
		//重新安装信号 并启动闹钟  无论是上传还是下载 都要调用start_data_alarm()
		start_data_alarm();
	}
	return ret;   //若是执行到这块了   说明建立数据连接套接字成功
}


static void do_list(session_t *sess)
{
	//创建数据连接
	if(get_transferfd(sess)==0)//创建数据连接失败	
		return ;
	//150
	ftp_reply(sess,FTP_DATACONN,"Here comes the directory listing.");
	//传输列表
	list_common(sess,1);//1表示完整的清单
	//226
	ftp_reply(sess,FTP_TRANSFEROK,"Directory send OK.");
	//关闭数据套接字
	close(sess->data_fd);
	sess->data_fd=-1;
}

static void do_nlst(session_t *sess)
{
	//创建数据连接
	if(get_transferfd(sess)==0)//创建数据连接失败	
		return ;
	//150
	ftp_reply(sess,FTP_DATACONN,"Here comes the directory listing.");
	//传输列表
	list_common(sess,0);  //0表示短的清单
	//226
	ftp_reply(sess,FTP_TRANSFEROK,"Directory send OK.");
	//关闭数据套接字
	close(sess->data_fd);
	sess->data_fd=-1;
}

///////////////////////////////////////////////////////////////////
static void do_cwd(session_t *sess) //改变当前目录  或者进入某个目录 或者返回上层目录
{
	if(chdir(sess->arg)<0)//如果登录的用户权限不够  则会failed
	{
		ftp_reply(sess,FTP_FILEFAIL,"failed to change directory."); 
		return ;
	}
	ftp_reply(sess,FTP_CWDOK,"Directory successfully changed."); 
}

static void do_cdup(session_t *sess)//返回上一层目录
{
	if(chdir("..")<0)//如果登录的用户权限不够  则会failed
	{
		ftp_reply(sess,FTP_FILEFAIL,"failed to change directory."); //550
		return ;
	}
	ftp_reply(sess,FTP_CWDOK,"Directory successfully changed."); 
}

static void do_mkd(session_t *sess)
{
	//实际创建的文件的权限是 0777 & umask
	if(mkdir(sess->arg,0777)<0)//在当前目录下创建文件
	{
		//失败的情形是: 在一个没有写权限的目录下创建目录会失败？？？？？？？？？？？？？？？？
		ftp_reply(sess,FTP_FILEFAIL,"Create directory operation failed.");//550
		return ;
	}
	char text[4096]={0};
	if(sess->arg[0]=='/')//绝对路径
	{
		//printf("%s\n",sess->arg);
		sprintf(text,"%s created",sess->arg);
	}
	else //相对路径
	{
		char dir[4096+1]={0};
		getcwd(dir,4096);		//获取ftp客户端登录服务器时的当前目录
		if(dir[strlen(dir)-1]=='/')//dir最后一个字符是否是/
		{
			sprintf(text,"%s%s created",dir,sess->arg);
		}
		else
			sprintf(text,"%s/%s created",dir,sess->arg);
	}
	
	ftp_reply(sess,FTP_MKDIROK,text);//550
}

//删除文件夹
//可以递归删除指定目录下的所有目录及文件  原因是: ftp客户端不断发送 到指定目录下的其他目录的命令 进而实现对应的删除
static void do_rmd(session_t *sess)
{
	if(rmdir(sess->arg)<0)
	{
		//失败的情形是: 在一个没有写权限的目录下删除目录会失败？？？？？？？？？？？？？？？？
		ftp_reply(sess,FTP_FILEFAIL,"Remove directory operation failed.");//550
		return ;
	}
	ftp_reply(sess,FTP_RMDIROK,"Remove directory operation successful.");
}

static void do_dele(session_t *sess)
{
	if(unlink(sess->arg)<0) //删除文件
	{
		ftp_reply(sess,FTP_FILEFAIL,"Delete operation failed.");//550
		return ;
	}
	ftp_reply(sess,FTP_DELEOK,"Delete operation sucessfully.");
}

static void do_rest(session_t *sess)
{
	//保存断点续传的位置信息
	sess->restart_pos=str_to_longlong(sess->arg);
	char text[1024]={0};
	sprintf(text,"Restart position accepted (%lld)",sess->restart_pos);
	ftp_reply(sess,FTP_RESTOK,text);
}
static void do_size(session_t *sess)
{
	struct stat buf;
	if(stat(sess->arg,&buf)<0)
	{
		ftp_reply(sess,FTP_FILEFAIL,"SIZE operation failed.");//550
		return ;
	}
	//如果不是普通文件的话
	//先判断是否是普通文件
	if (!S_ISREG(buf.st_mode))
	{	//如果不是普通文件 则响应以下信息
		ftp_reply(sess,FTP_FILEFAIL,"Could not get file size.");//550
		return ;
	}
	else
	{
		char text[1024]={0};
		sprintf(text,"%lld",(long long)buf.st_size);//获取普通文件的大小
		ftp_reply(sess,FTP_SIZEOK,text);
	}
}

static void do_rnfr(session_t *sess)
{
	sess->rnfr_name=(char *)malloc(strlen(sess->arg)+1);
	memset(sess->rnfr_name,0,strlen(sess->arg)+1);
	strcpy(sess->rnfr_name,sess->arg);
	ftp_reply(sess,FTP_RNFROK,"Ready for RNTO.");
}
static void do_rnto(session_t *sess)
{
	if(sess->rnfr_name==NULL)//之前没有收到RNFR命令
	{
		ftp_reply(sess,FTP_NEEDRNFR,"RNFR required first.");
		return ;
	}
	rename(sess->rnfr_name,sess->arg);
	ftp_reply(sess,FTP_RENAMEOK,"Rename successful.");
	free(sess->rnfr_name);
	sess->rnfr_name=NULL;
}

static void do_retr(session_t *sess)
{
	//下载文件   断点续传   客户端首先会发送PASV or PORT命令
	//创建数据连接
	if(get_transferfd(sess)==0)//创建数据连接失败	
		return ;
	long long offset=sess->restart_pos;
	sess->restart_pos=0;
	//先打开要下载的文件
	int fd=open(sess->arg,O_RDONLY);
	if(fd==-1)
	{
		ftp_reply(sess,FTP_FILEFAIL,"Failed to open file.");
		return ;
	}
	//给要打开的文件加读锁
	int ret=lock_file_read(fd);
	if(ret==-1)
	{
		ftp_reply(sess,FTP_FILEFAIL,"Lock file failed.");
		return ;
	}
	//由于设备文件不能被下载  所以要判断文件是否是普通文件
	struct stat sbuf;
	ret=fstat(fd,&sbuf);
	if(!S_ISREG(sbuf.st_mode))//如果不是普通文件
	{
		ftp_reply(sess,FTP_FILEFAIL,"The file is not a common file.");
		return ;
	}
	//lseek() 函数将与文件描述符fd关联的打开文件的偏移量重新定位到参数偏移量
	if(offset!=0)
	{
		ret=lseek(fd,offset,SEEK_SET);//SEEK_SET :偏移量设置为偏移字节
		if(ret<0)
		{
			ftp_reply(sess,FTP_FILEFAIL,"DingWei file failed!");
			return ;
		}
	}
	//150
	char text[1024];
	if(sess->is_ascii)
	{
		sprintf(text,"Opening ASCII mode data connection for %s (%lld)",
			sess->arg,(long long)sbuf.st_size);
	}
	else
	{
		sprintf(text,"Opening BINARY mode data connection for %s (%lld)",
			sess->arg,(long long)sbuf.st_size);
	}
	ftp_reply(sess,FTP_DATACONN,text);//150

	int flag=0;
	//下载文件
	//方法1   效率不高   原因: read write函数调用属于系统调用
	//都会涉及到用户空间与内核空间的的数据拷贝   效率不高
	/*
	int flag=0;//标志位 表示对应的几种情况
	char buf[4096]={0};
	while (1)
	{
		ret=read(fd,buf,sizeof(buf));
		if(ret==-1)
		{
			if(errno==EINTR)
				continue;
			else
			{
				flag=1;//表示错误
				break;
			}
		}
		else if(ret==0)//表示读取到了文件末尾
		{
			flag=0;   //
			break;
		}
	   //读取到了一定的数据  写到数据套接字里
		if (writen(sess->data_fd,buf,ret) != ret)
		{
			flag=2;
			break;
		}
	}
	*/
	//方法2
	long long bytes_to_send=sbuf.st_size;
	if(offset >  bytes_to_send) //断点的位置大于整个文件的大小  说明有错误
	{
		bytes_to_send=0;
	}
	else
		bytes_to_send-=offset;
	
	sess->bw_transfer_start_sec = get_time_sec();
	sess->bw_transfer_start_usec = get_time_usec();
	while(bytes_to_send)
	{
		int num_this_time=bytes_to_send > 4096 ? 4096 : bytes_to_send;

		ret=sendfile(sess->data_fd,fd,NULL,num_this_time);
		if(ret==-1)   //不考虑errno==EINTR的情况  因为sendfile()是在内核中进行拷贝数据的
		{
			flag=2;
			break;
		}
		limit_rate(sess,ret,0);
		bytes_to_send -= ret;
	}
	if (bytes_to_send==0)
	{
		flag=0;
	}
	//226的应答
	ftp_reply(sess,FTP_TRANSFEROK,"Directory send OK.");

	//关闭数据套接字
	close(sess->data_fd);
	sess->data_fd=-1;
	close(fd);
	
	if(flag==0)
	{
		//226
		ftp_reply(sess,FTP_TRANSFEROK,"Transfer complete.");
	}
	else if(flag==1)
	{
		//426
		ftp_reply(sess,FTP_BADSENDNET,"Failure reading from local file.");
	}
	else if(flag==2)
	{
		//451
       ftp_reply(sess,FTP_BADSENDFILE,"Failure writing to network stream .");
	}

	start_cmdio_alarm();//重新开启控制连接通道闹钟	原因是 之前的闹钟有可能关闭
}

//上传文件   断点续传   客户端首先会发送PASV or PORT命令
static void do_stor(session_t *sess)
{
	/*
	STOR  RETR
	REST  REST
	STOR  RETR
	APPE
	*/
	upload_common(sess,0);
}
/////////////////////////////////////////////////////////////////////
static void do_noop(session_t *sess) //该函数  可以防止空闲断开
{
	ftp_reply(sess,FTP_NOOPOK,"NOOP ok.");
}

static void do_quit(session_t *sess)
{
	ftp_reply(sess,FTP_GOODBYE,"Goodbye.");
	exit(EXIT_SUCCESS);
}
/*
该函数是将正处于数据传输的通道给断开  而不会断开控制连接通道
*/
static void do_abor(session_t *sess)
{

}
