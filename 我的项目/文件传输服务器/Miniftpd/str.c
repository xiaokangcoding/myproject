#include "str.h"
#include "common.h"

void str_trim_crlf(char *str)//去除\r\n
{
	char *p=&str[strlen(str)-1];
	while(*p=='\r' || *p=='\n')
	{
		*p='\0';
		p--;
	}

}

void str_split(const char *str , char *left, char *right, char c)
{
	//user root
	char *p=strchr(str,c);
	if(p==NULL)
		strcpy(left,str);
	else
	{
		strncpy(left,str,p-str);
		strcpy(right,p+1);
	}
}
//判断str是否全部是空格  是 则返回1 否则返回0
int str_all_space(const char *str)
{
	//const char *p=str;
	while(*str)
	{
		if(!isspace(*str))
			return 0;
		str++;
	}
	return 1;
}
//将字符串中的字符全部转换成大写
void str_upper(char *str)
{
	while(*str!='\0')
	{
		*str=toupper(*str);
		str++;
	}
}
long long str_to_longlong(const char *str)
{
	//atoll() 将字符串转换成long long 型整数
	long long result=0;
	long long int mul=1;
	int len=strlen(str);
	
	int i;
	long long val;
	for(i=len-1;i>=0;i--)
	{
		if(str[i]<'0' || str[i]>'9')
			return 0;
		val=str[i]-'0';
		val*=mul;
		mul*=10;
		result+=val;
	}
	return result;
}
//将八进制的字符串转换成unsigned int 类型

unsigned int str_octal_to_uint(const char *str)
{
	unsigned int result=0;
	int non_zero_digit=0;
	while(*str)
	{
		char digit=*str;
		if(digit<'0' || digit>'7')
			return 0;
		if(digit!='0')
			non_zero_digit=1;
		if(non_zero_digit)
		{
			//result *=8;
			result <<= 3;
			result+=(digit-'0');
		}
		str++;
	}
	return result;
}
/*
unsigned int str_octal_to_uint(const char *str)
{
	unsigned int result = 0;
	int seen_non_zero_digit = 0;

	while (*str) {
		int digit = *str;
		if (!isdigit(digit) || digit > '7')
			break;

		if (digit != '0')
			seen_non_zero_digit = 1;

		if (seen_non_zero_digit) {
			result <<= 3;
			result += (digit - '0');
		}
		str++;
	}
	return result;
}

*/

