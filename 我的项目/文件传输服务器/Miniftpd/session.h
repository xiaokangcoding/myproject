#ifndef _SESSION_H_
#define _SESSION_H_

#include "common.h"

//�ýṹ�屣��һЩ��ǰ�Ự����Ҫ��һЩ����
typedef struct session
{
	//������������Ҫ��һЩ����
	uid_t uid;
	int ctrl_fd;//�������ӵ��׽��� �����ӵ��׽��� conn
	char cmdline[MAX_COMMAND_LINE];
	char cmd[MAX_COMMAND];
	char arg[MAX_ARG];
	//��������
	struct sockaddr_in *port_addr;
	int pasv_listenfd;//�ж��Ƿ���pasv_active()״̬---�����׽���
	
	//���������׽���
	int data_fd;
	int data_process;
	//����
	unsigned int bw_upload_rate_max;//�ϴ����������
	unsigned int bw_download_rate_max;//���ص��������
	long bw_transfer_start_sec;//��ʼ�Ĵ���ʱ��(s)
	long bw_transfer_start_usec;//��ʼ�Ĵ���ʱ��(us)΢��
	//���ӽ��̵�ͨ��
	int parent_fd,child_fd;
	//FTPЭ��״̬
	int is_ascii;
	long long restart_pos;//���ڶϵ�����
	char * rnfr_name; 
	int abor_received;
	//������������
	unsigned int num_clients;
	unsigned int num_this_ip;
}session_t;
void begin_session(session_t *sess);
#endif