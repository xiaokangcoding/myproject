#include<stdio.h>
#include<stdlib.h>
int  main()
{
#if 0
	printf("Content_Type: text/html\n\n");
	printf("<h1 align=center />");
	printf("hello bigint");
	printf("</h1>");
	char *length=getenv("CONTENT_LENGTH");	
	int len=atoi(length);
	printf("len=%d",len);
#endif
#if 1
	char *data=getenv("QUERY_STRING");
	char m[10],n[10];
	sscanf(data,"m=%s",m);
	data=m+2;
	*(m+1)='\0';
	sscanf(data,"n=%s",n);
	*(n+1)='\0';
	printf("%s",m);
	printf("%s",n);
	int m1=atoi(m);
	int m2=atoi(n);
	int res=m1+m2;
	printf("res=%d",res);
//	printf(data);
#endif
	return 0;
}
