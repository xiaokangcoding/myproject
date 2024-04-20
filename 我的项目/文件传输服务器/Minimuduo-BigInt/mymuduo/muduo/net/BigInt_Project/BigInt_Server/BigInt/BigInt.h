#pragma once

#include"utili.h"
#include"SeqList.h"

class BigInt;
ostream& operator<<(ostream &out, const BigInt &bt);

class BigInt
{
	friend ostream& operator<<(ostream &out, const BigInt &bt);
public:
	BigInt(long value = 0);
	BigInt& operator=(u_long value);
public:
	void LoadData(size_t sz);
	void PrintData()const;
public:
	void Clear();
    	void BigIntCopy(char *buf, size_t len, size_t &pos);
public:
	u_char back()const;
	void pop_back();
	size_t size()const;
	void clear();
	void clear_head_zero();
	void push_back(u_char x);
	void push_front(u_char x);
public:
	void SetSymbol(Symbol sign);
	Symbol GetSymbol()const;
public:
	static u_char AddItem(u_char a, u_char b, u_char &sign);
	static u_char SubItem(u_char a, u_char b, u_char &sign);
	static void   MulItem(BigInt &bt, const BigInt &bt1, u_char x);
	static void   AddMove(BigInt &bt, const BigInt &bt1, int offset);
public:
	//bt = bt1 + bt2;
	static void Add(BigInt &bt, const BigInt &bt1, const BigInt &bt2);
	static void Sub(BigInt &bt, const BigInt &bt1, const BigInt &bt2);
	static void Mul(BigInt &bt, const BigInt &bt1, const BigInt &bt2);
	static void Div(BigInt &bt, const BigInt &bt1, const BigInt &bt2);
	static void Mod(BigInt &bt, const BigInt &bt1, const BigInt &bt2);
	static void Square(BigInt &bt, const BigInt &bt1);//bt = bt1^2;
	static void Power(BigInt &bt, const BigInt &bt1, u_long n);// bt = bt1^n;
	static void Power(BigInt &bt, const BigInt &bt1, const BigInt &bt2);//bt = bt1^bt2
	static void PowMod(BigInt &bt, BigInt &a, BigInt &b, const BigInt &n); //bt = bt1^bt2%bt3 //RSA
public:
	u_char& operator[](int pos);
	const u_char& operator[](int pos)const;
public:
	BigInt operator+(const BigInt &bt);
	BigInt operator-(const BigInt &bt);
	BigInt operator*(const BigInt &bt);
	BigInt operator/(const BigInt &bt);
	BigInt operator%(const BigInt &bt);
	BigInt operator^(const BigInt &bt);
public:
	BigInt& operator++();
	BigInt operator++(int);
	BigInt& operator--();
	BigInt operator--(int);
public:
	BigInt& operator+=(const BigInt &bt);
	BigInt& operator-=(const BigInt &bt);
	BigInt& operator*=(const BigInt &bt);
	BigInt& operator/=(const BigInt &bt);
	BigInt& operator%=(const BigInt &bt);
public:
	bool operator>(const BigInt &bt)const;
	bool operator<(const BigInt &bt)const;
	bool operator>=(const BigInt &bt)const;
	bool operator<=(const BigInt &bt)const;
	bool operator==(const BigInt &bt)const;
	bool operator!=(const BigInt &bt)const;
public:
	bool operator>(u_long x)const;
	bool operator&(u_long x);
private:
	SeqList<u_char> big;
};
