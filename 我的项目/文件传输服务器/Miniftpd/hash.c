#include "hash.h"
#include "common.h"
#include <assert.h>
//链表节点的数据结构
typedef struct hash_node {
	void *key;  
	void *value;
	struct hash_node *prev;
	struct hash_node *next;
} hash_node_t;

struct hash {
	unsigned int buckets;  //桶的个数
	hashfunc_t hash_func;  //哈希函数
	hash_node_t **nodes;   //哈希表中存放的一些链表(头指针的地址)
};

hash_node_t** hash_get_bucket(hash_t *hash, void *key);
hash_node_t* hash_get_node_by_key(hash_t *hash, void *key, unsigned int key_size);

hash_t* hash_alloc(unsigned int buckets, hashfunc_t hash_func)//创建一个哈希表
{
	hash_t *hash=(hash_t*)malloc(sizeof(hash_t));
	assert(hash!=NULL);
	hash->buckets=buckets;
	hash->hash_func=hash_func;

	int size=buckets*sizeof(hash_node_t *);;
	hash->nodes=(hash_node_t **)malloc(size);
	memset(hash->nodes, 0, size);
	return hash;
}

void* hash_lookup_entry(hash_t *hash, void* key, unsigned int key_size) //查找
{
	hash_node_t *node=hash_get_node_by_key(hash,key,key_size);
	if(node==NULL)
		return NULL;
	return node->value;
}
//添加一个数据项
void hash_add_entry(hash_t *hash, void *key, unsigned int key_size,void *value, unsigned int value_size)
{
	//先查找 如果该关键码存在 则不需要添加
	if(hash_lookup_entry(hash,key,key_size))
	{
		fprintf(stderr,"duplicate hash key\n");
		return;
	}
	hash_node_t *node=(hash_node_t *)malloc(sizeof(hash_node_t));
	node->key=malloc(key_size);
	memcpy(node->key,key,key_size);

	node->value=malloc(value_size);
	memcpy(node->value,value,value_size);
	node->prev=node->next=NULL; 

	hash_node_t **bucket=hash_get_bucket(hash,key);
	if(*bucket==NULL)
		*bucket=node;
	else
	{
		// 将新结点插入到链表头部
		node->next=*bucket;
		(*bucket)->prev=node;
		*bucket=node;
	}
}
//释放一个结点
void hash_free_entry(hash_t *hash, void *key, unsigned int key_size)
{
	hash_node_t *node=hash_get_node_by_key(hash,key,key_size);
	if(node==NULL)
		return ;
	free(node->key);
	free(node->value);
	if(node->prev)
		node->prev->next=node->next;
	else //头节点
	{
		hash_node_t **bucket=hash_get_bucket(hash,key);
		*bucket=node->next;
	}
	if(node->next)
		node->next->prev=node->prev;
	free(node);
}

hash_node_t** hash_get_bucket(hash_t *hash, void *key)//找桶号
{
	unsigned int bucket=hash->hash_func(hash->buckets,key);
	if(bucket>hash->buckets)
	{
		fprintf(stderr,"bad bucket lookup\n");
		exit(EXIT_FAILURE);
	}
	return &(hash->nodes[bucket]);
}

//根据关键码key查找某一结点
hash_node_t* hash_get_node_by_key(hash_t *hash, void *key, unsigned int key_size)
{
	hash_node_t **bucket = hash_get_bucket(hash,key);
	hash_node_t *node=*bucket;
	if(node==NULL)
		return NULL;
	while(node!=NULL && memcmp(node->key,key,key_size)!=0)
		node = node->next;
	return node;
}