#pragma once

#include"utili.h"

class BigInt;

template<class Type>
class SeqList
{
    friend class BigInt;
public:
	SeqList(size_t sz = DEFAULT_SIZE);
	SeqList(const SeqList<Type> &bt);
	SeqList& operator=(const SeqList<Type> &st);
	~SeqList();
public:
	typedef Type& reference;
	typedef const Type& const_reference;
public:
	void push_back(const Type &x);
	void push_front(const Type &x);
	void pop_back();
	void pop_front();
	reference front();
	reference back();
	const_reference back()const;
	const_reference front()const;
	size_t size()const;
	void reverse();
	void clear();
	void sort();
	int find(const Type &key);
	void erase(const Type &key);
public:
	reference operator[](int pos);
	const_reference operator[](int pos)const;
public:
	bool Inc()
	{
		capacity += INC_SIZE;
		Type *new_base = new Type[capacity+1];
		if(NULL == new_base)
		{
			cout<<"Out Of Memory."<<endl;
			//exit(1);
			return false;
		}
		memset(new_base, 0, sizeof(Type)*(capacity+1));
		memcpy(new_base, base, sizeof(Type)*(capacity-INC_SIZE+1));
		delete []base;
		base = new_base;
		return true;
	}

public:
	void SetSymbol(Symbol sign);
	Symbol GetSymbol()const;
private:
	enum{DEFAULT_SIZE = 10, INC_SIZE = 5};
	Type   *base;
	size_t capacity;
	size_t len;
};
///////////////////////////////////////////////////////////////
template<class Type>
SeqList<Type>::SeqList(size_t sz)
{
	capacity = sz > DEFAULT_SIZE ? sz : DEFAULT_SIZE;
	base = new Type[capacity+1];
	memset(base, 0, sizeof(Type) * (capacity+1));
	len = 0;
}
template<class Type>
SeqList<Type>::SeqList(const SeqList<Type> &bt)
{
	capacity = bt.capacity;
	base = new Type[sizeof(Type)*(capacity+1)];
	memset(base, 0, sizeof(Type)*(capacity+1));
	memcpy(base, bt.base, sizeof(Type)*(bt.len+1));
	len = bt.len;
}
//bt = bt1;
template<class Type>
SeqList<Type>& SeqList<Type>::operator=(const SeqList<Type> &st)
{
	if(this != &st)
	{
		if(capacity < st.len)
		{
			delete []base;
			base = new Type[st.capacity+1];
			capacity = st.capacity;	
		}
		memcpy(base, st.base, sizeof(Type)*(st.len+1));
		len = st.len;
	}
	return *this;
}
template<class Type>
SeqList<Type>::~SeqList()
{
	delete []base;
	base = NULL;
	capacity = len = 0;
}

template<class Type>
void SeqList<Type>::SetSymbol(Symbol sign)
{
	base[0] = sign;
}
template<class Type>
Symbol SeqList<Type>::GetSymbol()const
{
	return (Symbol)base[0];
}

template<class Type>
void SeqList<Type>::push_back(const Type &x)
{
	//assert(len < capacity);
	if(len>=capacity && !Inc())
		return;

	base[++len] = x;
}
template<class Type>
void SeqList<Type>::push_front(const Type &x)
{
	//assert(len < capacity);
	if(len>=capacity && !Inc())
		return;
	for(int i=++len; i>1; --i)
	{
		base[i] = base[i-1];
	}
	base[1] = x;
}

template<class Type>
void SeqList<Type>::pop_back()
{
	assert(len > 0);
	--len;
}
template<class Type>
void SeqList<Type>::pop_front()
{
	assert(len > 0);
	for(int i=1; i<len; ++i)
	{
		base[i] = base[i+1];
	}
	--len;
}

template<class Type>
typename SeqList<Type>::reference SeqList<Type>::front()
{
	assert(len > 0);
	return base[1];
}
template<class Type>
typename SeqList<Type>::reference SeqList<Type>::back()
{
	assert(len > 0);
	return base[len];
}
template<class Type>
typename SeqList<Type>::const_reference SeqList<Type>::front()const
{
	assert(len > 0);
	return base[1];
}
template<class Type>
typename SeqList<Type>::const_reference SeqList<Type>::back()const
{
	assert(len > 0);
	return base[len];
}
template<class Type>
size_t SeqList<Type>::size()const
{
	return len;
}
template<class Type>
void SeqList<Type>::reverse()
{
	if(len == 1)
		return;
	int low = 1;
	int high = len;
	Type tmp;
	while(low < high)
	{
		tmp = base[low];
		base[low] = base[high];
		base[high] = tmp;
		low++;
		high--;
	}
}
template<class Type>
void SeqList<Type>::clear()
{
	len = 0;
}
template<class Type>
void SeqList<Type>::sort()
{
	if(len == 1)
		return;

	for(int i=0; i<len-1; ++i)
	{
		for(int j=1; j<=len-i-1; ++j)
		{
			if(base[j] > base[j+1])
			{
				Type tmp = base[j];
				base[j] = base[j+1];
				base[j+1] = tmp;
			}
		}
	}
}

template<class Type>
int SeqList<Type>::find(const Type &key)
{
	for(int i=1; i<=len; ++i)
	{
		if(key == base[i])
			return i;
	}
	return -1;
}
template<class Type>
void SeqList<Type>::erase(const Type &key)
{
	int pos = find(key);
	if(pos == -1)
		return;
	for(int i=pos; i<len; ++i)
	{
		base[i] = base[i+1];
	}
	--len;
}
template<class Type>
typename SeqList<Type>::reference SeqList<Type>::operator[](int pos)
{
	assert(pos>=1 && pos<=len);
	return base[pos];
}
template<class Type>
typename SeqList<Type>::const_reference SeqList<Type>::operator[](int pos)const
{
	assert(pos>=1 && pos<=len);
	return base[pos];
}
