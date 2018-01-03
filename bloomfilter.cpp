#include"bloomfilter.h"
#include"measure.h"
#include<math.h>
#include<stdio.h>
#include<string.h>
#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif

static FORCE_INLINE uint32_t rotl32 ( uint32_t x, int8_t r )
{
	return (x << r) | (x >> (32 - r));
}

static FORCE_INLINE uint64_t rotl64 ( uint64_t x, int8_t r )
{
	return (x << r) | (x >> (64 - r));
}

#define	ROTL32(x,y)	rotl32(x,y)
#define ROTL64(x,y)	rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)

//-----------------------------------------------------------------------------
//// Block read - if your platform needs to do endian-swapping or can only
//// handle aligned reads, do the conversion here
//
#define getblock(p, i) (p[i])
//

static FORCE_INLINE uint32_t fmix32 ( uint32_t h )
{
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

	return h;
}
static FORCE_INLINE uint64_t fmix64 ( uint64_t k )
{
	k ^= k >> 33;
	k *= BIG_CONSTANT(0xff51afd7ed558ccd);
	k ^= k >> 33;
	k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
	k ^= k >> 33;

	return k;
}
extern MeasureTime mem;

void MurmurHash3_x86_32( const void * key, int len,uint32_t seed, void * out )
{
	const uint8_t * data = (const uint8_t*)key;
	const int nblocks = len / 4;
	int i;

	uint32_t h1 = seed;

	uint32_t c1 = 0xcc9e2d51;
	uint32_t c2 = 0x1b873593;
	const uint32_t * blocks = (const uint32_t *)(data + nblocks*4);

	for(i = -nblocks; i; i++)
	{
		uint32_t k1 = getblock(blocks,i);

		k1 *= c1;
		k1 = ROTL32(k1,15);
		k1 *= c2;

		h1 ^= k1;
		h1 = ROTL32(h1,13); 
		h1 = h1*5+0xe6546b64;
	}
	const uint8_t * tail = (const uint8_t*)(data + nblocks*4);

	uint32_t k1 = 0;

	switch(len & 3)
	{
		case 3: k1 ^= tail[2] << 16;
		case 2: k1 ^= tail[1] << 8;
		case 1: k1 ^= tail[0];
				k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
	}; h1 ^= len;

	h1 = fmix32(h1);

	*(uint32_t*)out = h1;
} 

KEYT hashfunction(KEYT key){
	key = ~key + (key << 15); // key = (key << 15) - key - 1;
	key = key ^ (key >> 12);
	key = key + (key << 2);
	key = key ^ (key >> 4);
	key = key * 2057; // key = (key + (key << 3)) + (key << 11);
	key = key ^ (key >> 16);
	return key;
}
BF* bf_init(int entry, double fpr){
	BF *res=(BF*)malloc(sizeof(BF));
	res->n=entry;
	res->m=ceil((res->n * log(fpr)) / log(1.0 / (pow(2.0, log(2.0)))));
	res->k=round(log(2.0) * (double)res->m / res->n);
	int targetsize=res->m/8;
	if(res->m%8)
		targetsize++;
	res->body=(char*)malloc(targetsize);
	memset(res->body,0,targetsize);
	res->p=fpr;
	res->targetsize=targetsize;
	return res;
}

uint64_t bf_bits(int entry, double fpr){
	uint64_t n=entry;
	uint64_t m=ceil((n * log(fpr)) / log(1.0 / (pow(2.0, log(2.0)))));
	int targetsize=m/8;
	if(m%8)
		targetsize++;
	return targetsize;
}
void bf_set(BF *input, KEYT key){
	KEYT h;
	for(int i=1; i<=input->k; i++){
		//MurmurHash3_x86_32(&key,sizeof(key),i,&h);
		h=hashfunction((key+(i*i))*i);
		h%=input->m;
		int block=h/8;
		int offset=h%8;
		BITSET(input->body[block],offset);
	}
}

bool bf_check(BF* input, KEYT key){
	KEYT h;
	for(int i=1; i<=input->k; i++){
		//MurmurHash3_x86_32(&key,sizeof(key),i,&h);
		h=hashfunction((key+(i*i))*i);
		h%=input->m;
		int block=h/8;
		int offset=h%8;
		if(!BITGET(input->body[block],offset))
			return false;
	}
	return true;
}void bf_save(BF* input,int fd){
	if(!write(fd,&input->n,sizeof(input->n))){
		printf("not work\n");
	}
	if(!write(fd,&input->p,sizeof(input->p))){
		printf("not work\n");
	}
	if(!write(fd,input->body,input->targetsize)){
		printf("not work\n");
	}
}
BF *bf_load(int fd){
	int n;
	double p;
	if(!read(fd,&n,sizeof(n))){
		printf("not work\n");
	}
	if(!read(fd,&p,sizeof(p))){
		printf("not work\n");
	}
	BF *result=bf_init(n,p);
	if(!read(fd,result->body,result->targetsize)){
		printf("not work\n");
	}
	return result;
}

void bf_free(BF *input){
	free(input->body);
	free(input);
}
/*
   int main(){
   int check=0;
   printf("test\n");
   BF *test=bf_init(KEYN,0.01);
   for(int i=0; i<1024; i++){
   bf_set(test,i);
   }

   for(int i=0; i<100000; i++){
   if(bf_check(test,i)){
   printf("%d\n",i);
   check++;
   }
   }

   printf("%d\n",check);
   }*/
