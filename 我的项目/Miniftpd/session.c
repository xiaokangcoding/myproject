#include "session.h"
#include "ftpproto.h"
#include "privparent.h"
#include "privsock.h"
#include "sysutil.h"
void begin_session(session_t *sess)
{
	//���ڽ���֮���ͨ��
/*	int sockfds[2];
	int ret=socketpair(PF_UNIX,SOCK_STREAM,0,sockfds);
	if(ret==-1)
		ERR_EXIT("socketpair");
		*/
	activate_oobinline(sess->ctrl_fd); //�������ݵĽ���
	priv_sock_init(sess);
	//�ٴ��������� nobody����   ftp�������
	pid_t pid;
	pid=fork();
	if(pid<0)
		ERR_EXIT("fork");
	if(pid==0) //ftp�������(ֱ����ftp�ͻ��˽��н���  ����ftp�ͻ��˷��͹��������� ���Ҷ���Щ������д��� �����Ӧ��ftp�ͻ���)------��Ҫ����ftpЭ����ص�ͨ��ϸ��--ftpproto.c
	{			//ftp������� ����Ҫ�����������(��������:��Ҫ�������ftp�ͻ��˷��͹�������������) 
				//ҲҪ������������(��������:���ڴ������� ����˵:�ļ����� Ŀ¼�б�Ĵ���)
		priv_sock_set_child_context(sess);
		//close(sockfds[0]);//ftp������̼ȿ��Զ� Ҳ����д
		//sess->child_fd=sockfds[1];
		handle_child(sess);
	}
	else if(pid>0) //nobody����------������------privparent.c
	{	
		//����ǰ���̸ĳ�nobody����
		struct passwd *pw;
		pw=getpwnam("nobody");
		if(pw==NULL)
			return;
		//����ȸ��û�id �ͺ��п���û��Ȩ��ȥ�޸���id
		if(setegid(pw->pw_gid)<0)//�ȸ���id �ٸ��û�id
			ERR_EXIT("setegid");
		if(seteuid(pw->pw_uid)<0)
			ERR_EXIT("seteuid");
		//nobody���̽���ftp������̷�������һЩ�����Э��ftp�������������� 
		//��: ����ftp������̽�����ftp�ͻ���֮�����������
		//close(sockfds[1]);//nobody���̼ȿ��Զ� Ҳ����д
		//sess->parent_fd=sockfds[0];
		priv_sock_set_parent_context(sess);
		handle_parent(sess);
	}
	//priv_sock_close(sess);
}

