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
	//��nobody������Ӧ����Ȩ(��Ȩ��С��) �Ա��20�˿�
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
		//�������ӽ��̷�������������Ϣ
		char cmd;
		//read(sess->parent_fd,&cmd,1);
		cmd=priv_sock_get_cmd(sess->parent_fd); //����������ִ������ĺ���
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

		//�����ڲ�����-----������ftp��׼����
		//�����ڲ�����
	}
}


static void privop_pasv_get_data_sock(session_t *sess)
{
	/*
	nobody���̽���PRIV_SOCK_GET_DATA_SOCK����
	��һ������һ������  Ҳ����port
	����һ���ַ���      Ҳ����ip
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
	int fd=tcp_client(20);//��20�˿�
	if(fd==-1)
	{
		priv_sock_send_result(sess->parent_fd,PRIV_SOCK_RESULT_BAD); //����ʧ�ܵ�Ӧ��
		return;
	}

	if(connect_timeout(fd,&addr,tunable_connect_timeout)<0)
	{
		priv_sock_send_result(sess->parent_fd,PRIV_SOCK_RESULT_BAD); //����ʧ�ܵ�Ӧ��
		close(fd);
		return ;
	}
	//���ͳɹ���Ӧ��
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
	 sess->pasv_listenfd=tcp_server(NULL,0);//�����׽���
	 //0:��ʾ �������ڰ󶨵�ʱ���ȷ������һ���˿ں�
	 //����ͨ������getsockname()��ȡ�󶨵Ķ˿ں�
	 struct sockaddr_in localaddr;
	 socklen_t addrlen=sizeof(struct sockaddr);
	 if(getsockname(sess->pasv_listenfd,(struct sockaddr*)&localaddr,&addrlen)<0)//��ȡ���صĵ�ַ��Ϣ ������localaddr��
		 ERR_EXIT("getsockname");
	 unsigned short port=ntohs(localaddr.sin_port);//�������ֽ���ת��Ϊ�����ֽ���
	 priv_sock_send_int(sess->parent_fd,(int)port);//���͸�ftp�������
}
static void privop_pasv_accept(session_t *sess)
{
	int fd=accept_timeout(sess->pasv_listenfd,NULL,tunable_accept_timeout);//NULL��ʾ������Ҫ�ͻ��˵ĵ�ַ��Ϣ
	
	close(sess->pasv_listenfd);//����accept_timeout�����Ƿ�ɹ�  ��Ҫ�رռ����׽���pasv_listenfd
	sess->pasv_listenfd=-1;
	if(fd==-1)	
	{
		priv_sock_send_result(sess->parent_fd,PRIV_SOCK_RESULT_BAD); //����ʧ�ܵ�Ӧ��
		return ; 
	}
	priv_sock_send_result(sess->parent_fd,PRIV_SOCK_RESULT_OK); //����ʧ�ܵ�Ӧ��
	sess->data_fd=fd;
	priv_sock_send_fd(sess->parent_fd,fd); //���������׽���
	close(fd);  //ֱ�ӹر�fd ��Ϊnobody���̲�ֱ����ftp�ͻ��˽������ݴ���
}