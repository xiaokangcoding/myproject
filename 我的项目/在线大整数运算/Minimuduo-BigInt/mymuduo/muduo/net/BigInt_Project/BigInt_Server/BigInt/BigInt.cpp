
#include"BigInt.h"

ostream& operator<<(ostream &out, const BigInt &bt)
{
	if(bt.GetSymbol() == NEGATIVE)
		cout<<"-";
	for(size_t i=bt.size(); i>=1; --i)
	{
		out<<(int)bt[i];
	}
	return out;
}

void BigInt::SetSymbol(Symbol sign)
{
	big.SetSymbol(sign);
}
Symbol BigInt::GetSymbol()const
{
	return big.GetSymbol();
}

BigInt::BigInt(long value)
{
	SetSymbol(POSITIVE);
	if(value == 0)
	{
		push_back(0);
	}
	else
	{
		if(value < 0)
		{
			SetSymbol(NEGATIVE);
			value  = abs(value);
		}
		while(value > 0)
		{
			push_back(value % 10);
			value /= 10;
		}
	}
}

//bt = 123
BigInt& BigInt::operator=(u_long value)
{
	(*this).clear();
	if(value == 0)
	{
		push_back(0);
	}
	else
	{
		while(value > 0)
		{
			push_back(value % 10);
			value /= 10;
		}
	}
	return *this;
}
/////////////////////////////////////////////////////////
void BigInt::LoadData(size_t sz)
{
	clear();
	srand(time(NULL));
	for(int i=0; i<sz; ++i)
	{
		big.push_back(rand()%10);
	}
	clear_head_zero();
}
void BigInt::PrintData()const
{
	for(int i=size(); i>=1; --i)
	{
		cout<<(int)big[i];
	}
	cout<<endl;
}
////////////////////////////////////////////////////////////
void BigInt::Clear()
{
	big.clear();
}
void BigInt::BigIntCopy(char *buf, size_t len, size_t &pos)
{
	if(pos == 0)
	{
		memcpy(buf,big.base,1); //copy 0 index
		pos++;
	}
	memcpy(buf,big.base+pos, len);
}
////////////////////////////////////////////////////////////
u_char BigInt::back()const
{
	return big.back();
}
void BigInt::pop_back()
{
	big.pop_back();
}
size_t BigInt::size()const
{return big.size();}
void BigInt::clear()
{big.clear();}
void BigInt::push_back(u_char x)
{
	big.push_back(x);
}
void BigInt::push_front(u_char x)
{
	big.push_front(x);
}
void BigInt::clear_head_zero()
{
	while(size()>0 && *this!=0 && (*this).back() == 0)
	{
		pop_back();
	}
}
////////////////////////////////////////////////////////////
u_char& BigInt::operator[](int pos)
{return big[pos];}
const u_char& BigInt::operator[](int pos)const
{return big[pos];}
bool BigInt::operator>(const BigInt &bt)const
{
	if(size() > bt.size())
		return true;
	else if(size() < bt.size())
		return false;

	size_t i = size();
	while(i >= 1)
	{
		if((*this)[i] > bt[i])
			return true;
		else if((*this)[i] < bt[i])
			return false;
		--i;
	}
	return false;
}
bool BigInt::operator<=(const BigInt &bt)const
{
	return !(*this > bt);
}
bool BigInt::operator<(const BigInt &bt)const
{
	if(size() < bt.size())
		return true;
	else if(size() > bt.size())
		return false;

	size_t i = size();
	while(i >= 1)
	{
		if((*this)[i] < bt[i])
			return true;
		else if((*this)[i] > bt[i])
			return false;
		--i;
	}
	return false;
}
bool BigInt::operator>=(const BigInt &bt)const
{
	return !(*this < bt);
}

bool BigInt::operator==(const BigInt &bt)const
{
	if(size() != bt.size())
		return false;
	size_t i = 1;
	while(i <= size())
	{
		if((*this)[i] != bt[i])
			return false;
		++i;
	}
	return true;

}
bool BigInt::operator!=(const BigInt &bt)const
{
	return !(*this == bt);
}
////////////////////////////////////////////////////////////
bool BigInt::operator>(u_long x)const
{
	BigInt tmp(x);
	return *this > tmp;
}
////////////////////////////////////////////////////////////
BigInt& BigInt::operator++()
{
	u_char sign = 1;
	size_t i = 1;
	while(sign>0 && i<=size())
	{
		(*this)[i] = AddItem((*this)[i], 0, sign);
		++i;
	}
	if(sign > 0)
		push_back(sign);
	return *this;
}
BigInt BigInt::operator++(int)
{
	BigInt tmp = *this;
	++*this;
	return tmp;
}
BigInt& BigInt::operator--()
{
	assert(*this > 0);
	u_char sign = 1;
	size_t i = 1;
	while(sign>0 && i<=size())
	{
		(*this)[i] = SubItem((*this)[i], 0, sign);
		++i;
	}
	clear_head_zero();
	return *this;
}
BigInt BigInt::operator--(int)
{
	BigInt tmp = *this;
	--*this;
	return tmp;
}
////////////////////////////////////////////////////////////
//123  417491941949
BigInt& BigInt::operator+=(const BigInt &bt)
{
	u_char sign = 0;
	size_t i=1, j=1;
	while(i<=size() && j<=bt.size())
	{
		(*this)[i] = AddItem((*this)[i], bt[j], sign);
		++i;
		++j;
	}
	while(sign>0 && i<=size())
	{
		(*this)[i] = AddItem((*this)[i], 0, sign);
		++i;
	}
	while(j <= bt.size())
	{
		u_char sum = AddItem(0, bt[j], sign);
		push_back(sum);
		++j;
	}
	if(sign > 0)
		push_back(sign);
	return *this;

}
BigInt& BigInt::operator-=(const BigInt &bt)
{
	assert(*this >= bt);
	if(*this == bt)
	{
		*this = 0;
		return *this;
	}
	else
	{
		u_char sign = 0;
		size_t i=1, j=1;
		while(i<=size() && j<=bt.size())
		{
			(*this)[i] = SubItem((*this)[i], bt[j], sign);
			++i;
			++j;
		}
		while(sign>0 && i<=size())
		{
			(*this)[i] = SubItem((*this)[i], 0, sign);
			++i;
		}
	}
	return *this;
}
BigInt& BigInt::operator*=(const BigInt &bt)
{
	BigInt tmp;
	BigInt::Mul(tmp, *this, bt);
	*this = tmp;
	return *this;
}
BigInt& BigInt::operator/=(const BigInt &bt)
{
	BigInt tmp;
	BigInt::Div(tmp, *this, bt);
	*this = tmp;
	return *this;
}
BigInt& BigInt::operator%=(const BigInt &bt)
{
	BigInt tmp;
	BigInt::Mod(tmp, *this, bt);
	*this = tmp;
	return *this;
}
////////////////////////////////////////////////////////////
BigInt BigInt::operator+(const BigInt &bt)
{
	BigInt tmp;
	BigInt::Add(tmp, *this, bt);
	return tmp;
}
BigInt BigInt::operator-(const BigInt &bt)
{
	BigInt tmp;
	BigInt::Sub(tmp, *this, bt);
	return tmp;
}
BigInt BigInt::operator*(const BigInt &bt)
{
	BigInt tmp;
	BigInt::Mul(tmp, *this, bt);
	return tmp;
}
BigInt BigInt::operator/(const BigInt &bt)
{
	BigInt tmp;
	BigInt::Div(tmp, *this, bt);
	return tmp;
}
BigInt BigInt::operator%(const BigInt &bt)
{
	BigInt tmp;
	BigInt::Mod(tmp, *this, bt);
	return tmp;
}
BigInt BigInt::operator^(const BigInt &bt)
{
	BigInt tmp;
	BigInt::Power(tmp, *this, bt);
	return tmp;
}
////////////////////////////////////////////////////////////
bool BigInt::operator&(u_long x)
{
	BigInt tmp(2);
	BigInt ans = *this % tmp;
	return ans==1;
}
////////////////////////////////////////////////////////////
u_char BigInt::AddItem(u_char a, u_char b, u_char &sign)
{
	u_char sum = a + b + sign;
	if(sum >= 10)
	{
		sum -= 10;
		sign = 1;
	}
	else
		sign = 0;
	return sum;
}
void BigInt::Add(BigInt &bt, const BigInt &bt1, const BigInt &bt2)
{
	bt.clear();
	u_char sum, sign = 0;
	size_t i=1, j=1;
	while(i<=bt1.size() && j<=bt2.size())
	{
		sum = AddItem(bt1[i],bt2[j],sign);
		bt.push_back(sum);
		++i;
		++j;
	}
	while(i <= bt1.size())
	{
		sum = AddItem(bt1[i], 0, sign);
		bt.push_back(sum);
		++i;
	}
	while(j <= bt2.size())
	{
		sum = AddItem(0, bt2[j], sign);
		bt.push_back(sum);
		++j;
	}
	if(sign > 0)
		bt.push_back(sign);
}

u_char BigInt::SubItem(u_char a, u_char b, u_char &sign)
{
	u_char sub;
	if(a >= b+sign)
	{
		sub = a - b - sign;
		sign = 0;
	}
	else
	{
		sub = a + 10 - b - sign;
		sign = 1;
	}
	return sub;
}
void BigInt::Sub(BigInt &bt, const BigInt &bt1, const BigInt &bt2)
{
	bt.clear();
	if(bt1 < bt2)
		return;
	if(bt1 == bt2)
		bt = 0;

	u_char sub, sign = 0;
	size_t i=1, j=1;
	while(i<=bt1.size() && j<=bt2.size())
	{
		sub = SubItem(bt1[i], bt2[j], sign);
		bt.push_back(sub);
		++i;
		++j;
	}
	while(i<=bt1.size())
	{
		sub = SubItem(bt1[i], 0, sign);
		bt.push_back(sub);
		++i;
	}
	bt.clear_head_zero();
}
void BigInt::MulItem(BigInt &bt, const BigInt &bt1, u_char x)
{
	u_char mul, sign = 0;
	size_t i = 1;
	while(i <= bt1.size())
	{
		mul = bt1[i] * x + sign; //4  5
		if(mul >= 10)
		{
			sign = mul / 10;
			mul %= 10;
		}
		else
			sign = 0;
		bt.push_back(mul);
		++i;
	}
	if(sign > 0)
		bt.push_back(sign);
}
void BigInt::AddMove(BigInt &bt, const BigInt &bt1, int offset)
{
	u_char sign = 0;
	size_t i = offset;
	size_t j = 1;
	while(i<=bt.size() && j<=bt1.size())
	{
		bt[i] = AddItem(bt[i], bt1[j], sign);
		++i;
		++j;
	}
	/*
	while(sign>0 && i<=bt.size())
	{
		bt[i] = AddItem(bt[i], 0, sign);
		++i;
	}*/
	while(j <= bt1.size())
	{
		u_char sum = AddItem(0, bt1[j], sign);
		bt.push_back(sum);
		++j;
	}
	if(sign > 0)
		bt.push_back(sign);
}
void BigInt::Mul(BigInt &bt, const BigInt &bt1, const BigInt &bt2)
{
	BigInt tmp;
	for(size_t i=1; i<=bt2.size(); ++i)
	{
		tmp.clear();
		MulItem(tmp, bt1, bt2[i]);
		AddMove(bt, tmp, i);
	}
}
void BigInt::Div(BigInt &bt, const BigInt &bt1, const BigInt &bt2)
{
	bt = 0;
	if(bt1 < bt2)
		bt = 0;
	else if(bt1 == bt2)
		bt = 1;
	else
	{
		size_t bt1_len = bt1.size();
		size_t bt2_len = bt2.size();
		int k = bt1_len - bt2_len;

		BigInt btd;
		btd.clear();
		for(size_t i=1; i<=bt2.size(); ++i)
		{
			btd.push_back(bt1[i+k]);
		}
		u_char div = 0;
		while(k >= 0)
		{
			while(btd >= bt2)
			{
				btd -= bt2;
				div++;
				btd.clear_head_zero();
			}
			bt.push_front(div);
			div = 0;

			if(k > 0)
				btd.push_front(bt1[k]);
			--k;
		}
		bt.clear_head_zero();
	}
}
void BigInt::Mod(BigInt &bt, const BigInt &bt1, const BigInt &bt2)
{
	bt = 0;
	if(bt1 < bt2)
		bt = bt1;
	else if(bt1 == bt2)
		bt = 0;
	else
	{
		size_t bt1_len = bt1.size();
		size_t bt2_len = bt2.size();
		int k = bt1_len - bt2_len;

		BigInt btd;
		btd.clear();
		for(size_t i=1; i<=bt2.size(); ++i)
		{
			btd.push_back(bt1[i+k]);
		}

		while(k >= 0)
		{
			while(btd >= bt2)
			{
				btd -= bt2;
				btd.clear_head_zero();
			}

			if(k > 0)
				btd.push_front(bt1[k]);
			--k;
		}
		bt = btd;
		bt.clear_head_zero();
	}
}
void BigInt::Square(BigInt &bt, const BigInt &bt1)
{
	BigInt::Mul(bt, bt1, bt1);
}
void BigInt::Power(BigInt &bt, const BigInt &bt1, u_long n)
{
	bt = 1;
	for(u_long i=0; i<n; ++i)
	{
		bt *= bt1;
	}
}
void BigInt::Power(BigInt &bt, const BigInt &bt1, const BigInt &bt2)
{
	bt = 1;
	for(BigInt i=0; i<bt2; ++i)
	{
		bt *= bt1;
	}
}

//ACM   a^b%c
void BigInt::PowMod(BigInt &bt, BigInt &a, BigInt &b, const BigInt &n)
{
	BigInt ans = 1;
	a %= n;
	while(b != 0)
	{
		if(b & 1)
		{
			ans = (ans * a) % n;
		}
		a = (a*a) % n;
		b = b / 2;
	}
	bt = ans;
	bt.clear_head_zero();
}
/*
//RSA  bt = a^b % n
void BigInt::PowMod(BigInt &bt, BigInt &a, BigInt &b, const BigInt &n)
{
BigInt tmp;
BigInt::Power(tmp, a, b);
BigInt::Mod(bt, tmp, n);
}
*/
