#ifndef __DELETE_H__
#define __DELETE_H__
#include "utils.h"
#include<limits.h>
typedef struct{
	uint8_t bitset[PAGENUM/8];
	int number;
	int invalid_n;
}block;

typedef struct{
	block blocks[SEGNUM];
}delete_set;

void delete_init(delete_set *);
void delete_ppa(delete_set *set,KEYT ppa);
void delete_trim_process(delete_set *);
void delete_free(delete_set *);
int delete_get_victim(delete_set *);
inline uint64_t KEYSET(uint64_t des,uint32_t src){
	uint64_t temp=src;
	return des | (temp<<32);
}
inline uint64_t LEVELSET(uint64_t des, int level){
	uint64_t temp=level;
	return des | (level<<8);
}
inline uint64_t FLAGSET(uint64_t des, bool flag){
	return des | flag;
}
inline uint64_t KEYGET(uint64_t des){
	return des>>32;
}
inline uint64_t LEVELGET(uint64_t des){
	uint64_t temp=UCHAR_MAX;
	return des>>8 & temp;
}
inline bool FLAGGET(uint64_t des){
	return des & 1;
}
#endif