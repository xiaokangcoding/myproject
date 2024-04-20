#include<iostream>
#include<unistd.h>
#include<stdio.h>
#include"./BigInt/BigInt.h"
using namespace std;

typedef enum {ADD=1,SUB, MUL, DIV,MOD,Square,Pow,PowMod} OPER;

void Html_Header()
{
	printf("Content-Type:text/html;charset=utf-8\n\n");
	printf("<title>51CC BigInt</title>");
	printf("<H1 align=center>WelCome To 51CC.</H1>");
//	printf("<body background=img/calc.jpg> \
			style=" background-repeat:no-repeat ;background-size:100% 100%; background-attachment: fixed;">");
}
void Data_Handler(BigInt &bt, BigInt &bt1, BigInt &bt2, OPER &oper)
{
	bt.clear();
	bt1.clear();
	bt2.clear();

	//  char *data = getenv("QUERY_STRING");//采用GET时所传输的信息

	//printf("<p>data = %s<br>",data);
	int len = atoi(getenv("CONTENT_LENGTH"));
	char *data=(char*)malloc(len+1);
	fgets(data,len+1,stdin);

	while(*data!='\0' && *data!='&')
	{
		if(*data>='0' && *data<='9')
			bt1.push_front(*data-'0');
		++data;
	}
	data++;
	while(*data!='\0' && *data!='&')
	{
		if(*data>='0' && *data<='9')
			bt2.push_front(*data-'0');
		++data;
	}

	while(*data != '\0')
	{
		if(*data == '1')
		{
			oper = ADD;
			break;
		}
		else if(*data == '2')
		{
			oper = SUB;
			break;
		}
		else if(*data == '3')
		{
			oper = MUL;
			break;
		}
		else if(*data == '4')
		{
			oper = DIV;
			break;
		}
		else if(*data == '5')
		{
			oper = MOD;
			break;
		}
		else if(*data=='6')
		{
			oper=Square;
			break;
		}
		else if(*data=='7')
		{
			oper=Pow;
			break;
		}
		else
			data++;
	}


	///////////////////////////////////////////////////////////

	if(oper == ADD)
		bt = bt1 + bt2;
	else if(oper == SUB)
		bt = bt1 - bt2;
	else if(oper == MUL)
		bt = bt1 * bt2;
	else if(oper == DIV)
		bt = bt1 / bt2;
	else if(oper == MOD)
		bt = bt1 % bt2;
	else if(oper == Square)
		BigInt::Square(bt,bt1);
	else if(oper == Pow)
		bt = bt1 ^ bt2;
//	else if(oper == PowMod)
//		bt = (bt1 ^ bt2)%;
}

void Show_Result(const BigInt &bt, const BigInt &bt1, const BigInt &bt2, OPER oper)
{
	printf("<br>");
	printf("<div style='word-wrap:break-word;font-size:30px;font-weight:blod;' align=center>bt1=");
	for(int i=bt1.size(); i>=1; --i)
		printf("%d", bt1[i]);
	printf("<br>");

	if(oper == ADD)
		printf("+<br>");
	else if(oper == SUB)
		printf("-<br>");
	else if(oper == MUL)
		printf("*<br>");
	else if(oper == DIV)
		printf("/<br>");
	else if(oper == MOD)
		printf("%<br>");
	else if(oper == Square)
		printf("^2<br>");
	else if(oper == Pow)
		printf("^<br>");
//	else if(oper == PowMod)
//		printf("^%<br>");

	if(oper!=Square)
	{
		printf("bt2=");
		for(int i=bt2.size();i>=1; --i)
			printf("%d", bt2[i]);
	}
	printf("<br>");

	//printf("result = ");
	printf("||<br>");
	for(int i=bt.size(); i>=1; --i)
		printf("%d", bt[i]);
	printf("</div>");
}

int main(int argc, char* argv[])
{
	Html_Header();

	BigInt bt, bt1, bt2;
	OPER oper;
	Data_Handler(bt, bt1, bt2, oper);

	Show_Result(bt, bt1, bt2, oper);

	return 0;
}
















