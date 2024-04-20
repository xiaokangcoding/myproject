#include "parseconf.h"
#include "common.h"
#include "tunable.h"
#include "str.h"
/////////////////////////////////////////////////////////////////////////
static struct parseconf_bool_setting {
  const char *p_setting_name; //�����������
  int *p_variable;
}

parseconf_bool_array[] = {
	{ "tunable_pasv_enable", &tunable_pasv_enable },
	{ "tunable_port_enable", &tunable_port_enable },
	{ NULL, NULL }
};

static struct parseconf_uint_setting {
	const char *p_setting_name;
	unsigned int *p_variable;
}
parseconf_uint_array[] = {
	{ "tunable_listen_port", &tunable_listen_port },
	{ "tunable_max_clients", &tunable_max_clients },
	{ "tunable_max_per_ip", &tunable_max_per_ip },
	{ "tunable_accept_timeout", &tunable_accept_timeout },
	{ "tunable_connect_timeout", &tunable_connect_timeout },
	{ "tunable_idle_session_timeout", &tunable_idle_session_timeout },
	{ "tunable_data_connection_timeout", &tunable_data_connection_timeout },
	{ "tunable_local_umask", &tunable_local_umask },
	{ "tunable_upload_max_rate", &tunable_upload_max_rate },
	{ "tunable_download_max_rate", &tunable_download_max_rate },
	{ NULL, NULL }
};

static struct parseconf_str_setting {
	const char *p_setting_name;
	const char **p_variable;
}

parseconf_str_array[] = {
	{ "tunable_listen_address", &tunable_listen_address },
	{ NULL, NULL }
};
/////////////////////////////////////////////////////////////////////////

//���������ļ�
void parseconf_load_file(const char *path)
{
	FILE *fp=fopen(path,"r");
	if(fp==NULL)
		ERR_EXIT("fopen");
	//һ��һ�ж�ȡ�����ļ�������  ��������Ӧ�ı�����
	char setting_line[1024]={0};
	while(fgets(setting_line,sizeof(setting_line),fp)!=NULL)
	{
		if(strlen(setting_line)==0 || str_all_space(setting_line) || setting_line[0]=='#' )
			continue;
		str_trim_crlf(setting_line);//ȥ��\n  ��Ϊfgets()�����\n

		//���������� ���������������Ϣ��������Ӧ�ı�����
		parseconf_load_setting(setting_line);
		memset(setting_line,0,sizeof(setting_line));
	}
	fclose(fp);
}

//���������ļ��õ�����������Ϣ��������Ӧ�ı�����
void parseconf_load_setting(const char *setting)
{
	//ȥ����ո�
	while(isspace(*setting))
		setting++;
	char key[128]={0};//�������key
	char value[128]={0};//�������value
	str_split(setting,key,value,'=');
	if(strlen(value)==0)
	{
		fprintf(stderr,"missing value in the config file for %s\n",key);
		exit(EXIT_FAILURE);
	}
	////////////////////////////////////////////////////////////////////////////////////
	//�ַ���������
	const struct parseconf_str_setting * p_str_setting=parseconf_str_array;
	while(p_str_setting->p_setting_name!=NULL)
	{
		if( strcmp(p_str_setting->p_setting_name,key)==0 )//�ҵ���
		{
			const char **p_cur_setting=p_str_setting->p_variable;
			if(*p_cur_setting)
				free((char *)*p_cur_setting);//�ͷ�ԭ�е�����
			*p_cur_setting=strdup(value);//strdup()���� ��Ҫ�ǿ����ַ���value��һ������ ����������Լ����ڴ�ռ�(��malloc()����)
			return ;
		}
		p_str_setting++;
	}
	///////////////////////////////////////////////////////////////////////////////////////
	//����������
	//AA=YES
	const struct parseconf_bool_setting * p_bool_setting=parseconf_bool_array;
	while(p_bool_setting->p_setting_name!=NULL)
	{
		if( strcmp(p_bool_setting->p_setting_name,key)==0 )//�ҵ���
		{
			str_upper(value);
			if(strcmp(value,"YES")==0 ||
				strcmp(value,"1")==0 ||
				strcmp(value,"TRUE")==0 )
				*(p_bool_setting->p_variable)=1;
			else if(strcmp(value,"NO")==0 ||
				strcmp(value,"0")==0 ||
				strcmp(value,"FALSE")==0 )
				*(p_bool_setting->p_variable)=0;
			else //value���ǺϷ�����������Ϣ
			{
				fprintf(stderr,"missing value in the config file for %s\n",key);
				exit(EXIT_FAILURE);
			}
			return ;
		}
		p_bool_setting++;
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	//�޷���������   ע��:�޷������������ֿ���  һ��������  ��һ���ǰ˽��Ƶ���ʽ
	const struct parseconf_uint_setting * p_uint_setting=parseconf_uint_array;
	while(p_uint_setting->p_setting_name!=NULL)
	{
		if( strcmp(p_uint_setting->p_setting_name,key)==0 )//�ҵ���
		{
			if(value[0]=='0')
				*(p_uint_setting->p_variable)=str_octal_to_uint(value);//value���ַ���
			else
				*(p_uint_setting->p_variable)=atoi(value);
			return ;
		}
		p_uint_setting++;
	}
	//////////////////////////////////////////////////////////////////////////////////////////
}
