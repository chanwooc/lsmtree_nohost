#include"run_array.h"
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
Node *ns_run(level*input ,int n){
	return (Node*)&input->body[input->r_size*n];
}
Entry *ns_entry(Node *input, int n){
	if(input->m_num<n+1) return NULL;
	return (Entry*)(input->body_addr+n*input->e_size/sizeof(char*));
}

Entry *level_entcpy(Entry *src, char *des){
	memcpy(des,src,sizeof(Entry));
#ifdef BLOOM
	#ifdef MONKEY

	#else

	#endif
#endif
	return (Entry*)des;
}

level *level_init(level *input,int all_entry,bool isTiering){
	if(isTiering){
		input->r_num=MUL;
	}
	else{
		input->r_num=1;
	}
	input->isTiering=isTiering;
	int entry_p_run=all_entry/input->r_num;
	int run_body_size=sizeof(Entry)*entry_p_run;
	int run_size=sizeof(Node)-sizeof(char*)+run_body_size;
	int level_body_size=run_size*input->r_num;

	input->body=(char*)malloc(level_body_size);
	input->r_size=run_size;
	input->m_num=input->r_num*entry_p_run;
	input->n_num=0;

	for(int i=0; i<input->r_num; i++){
		Node *temp_run=ns_run(input,i);
		temp_run->n_num=0;
		temp_run->m_num=entry_p_run;
		temp_run->e_size=sizeof(Entry);
		temp_run->body_addr=&temp_run->body;
	}
	input->entry_p_run=entry_p_run;
	input->r_n_num=1;
	input->start=UINT_MAX;
	input->end=0;
	return input;
}
Entry **level_find(level *input,KEYT key){
	Entry **res=(Entry**)malloc(sizeof(Entry*)*input->r_n_num);
	bool check=false;
	int cnt=0;
	for(int i=0; i<input->r_num; i++){
		Node *run=ns_run(input,i);
		Entry *temp=level_find_fromR(run,key);
		if(temp){
			res[cnt++]=temp;
			check=true;
		}
	}
	if(!check){
		free(res);
		return NULL;
	}
	res[cnt]=NULL;
	return res;
}

Entry *level_find_fromR(Node *run, KEYT key){
	int start=0;
	int end=run->n_num;
	int mid;
	while(1){
		mid=(start+end)/2;
		Entry *mid_e=ns_entry(run,mid);
		if(mid_e==NULL) break;
		if(mid_e->key <=key && mid_e->end>=key){
#ifdef BLOOM
			if(!bf_check(mid_e->filter,key)){
				return NULL;
			}
#endif
			return mid_e;
		}
		if(mid_e->key>key){
			end=mid-1;
		}
		else if(mid_e->end<key){
			start=mid+1;
		}

		if(start>end)
			break;
	}
	return NULL;
}

Node *level_insert(level *input,Entry *entry){//always sequential
	if(input->start>entry->key)
		input->start=entry->key;
	if(input->end<entry->end)
		input->end=entry->end;

	if(input->n_num==input->m_num) return NULL;
	int r=input->n_num/input->entry_p_run;
	if(input->r_n_num==r)
		input->r_n_num++;
	Node *temp_run=ns_run(input,r);//active run
	int o=temp_run->n_num;
	Entry *temp_entry=ns_entry(temp_run,o);
	level_entcpy(entry,(char*)temp_entry);
	temp_run->n_num++;
	input->n_num++;
}
Entry *level_get_next(Iter * input){
	if(!input->flag) return NULL;
	if(input->now->n_num==0) return NULL;
	Entry *res=ns_entry(input->now,input->idx++);
	if(input->idx==input->now->n_num){
		if(input->lev->r_n_num == input->r_idx){
			input->flag=false;
		}
		else{
			input->r_idx++;
			input->now=ns_run(input->lev,input->r_idx);
			input->idx=0;
		}
	}
	return res;
}
Iter *level_get_Iter(level *input){
	Iter *res=(Iter*)malloc(sizeof(Iter));
	res->now=ns_run(input,0);
	res->idx=0;
	res->r_idx=0;
	res->lev=input;
	res->flag=true;
	return res;
}
void level_print(level *input){
	for(int i=0; i<input->r_n_num; i++){
		Node* temp_run=ns_run(input,i);
		for(int j=0; j<temp_run->n_num; j++){
			Entry *temp_ent=ns_entry(temp_run,j);
			printf("Key: %d, End: %d, Pbn: %d\n",temp_ent->key,temp_ent->end,temp_ent->pbn);
		}
	}
}
void level_free(level *input){
	free(input->body);	
	free(input);
}
level *level_clear(level *input){
	input->n_num=0;
	input->r_n_num=0;
	for(int i=0; i<input->r_num; i++){
		Node *temp_run=ns_run(input,i);
		temp_run->n_num=0;
	}
	input->start=UINT_MAX;
	input->end=0;
	return input;
}
Entry *level_make_entry(KEYT key,KEYT end,KEYT pbn){
	Entry *ent=(Entry *)malloc(sizeof(Entry));
	ent->key=key;
	ent->end=end;
	ent->pbn=pbn;
	memset(ent->bitset,0,sizeof(ent->bitset));
	return ent;
}
bool level_check_overlap(level *input ,KEYT start, KEYT end){
	if(input->start>end){
		return false;
	}
	if(input->end<start)
		return false;
	return true;
}
/*
int main(){
	level *temp_lev=(level*)malloc(sizeof(level));
	level_init(temp_lev,48,true);
	for(int i=0; i<48; i++){
		Entry *temp=level_make_entry(i,i,i);
		level_insert(temp_lev,temp);
		free(temp);
	}
	Iter *iter=level_get_Iter(temp_lev);
	Entry *temp_ent;
	while((temp_ent=level_get_next(iter))!=NULL){
		printf("Key: %d, End: %d, Pbn: %d\n",temp_ent->key,temp_ent->end,temp_ent->pbn);
	}

//	for(int i=0; i<48; i++){
//		Entry *temp_ent=level_find(temp_lev,i);
//		printf("Key: %d, End: %d, Pbn: %d\n",temp_ent->key,temp_ent->end,temp_ent->pbn);
//	}
	
//	printf("\n\n");
//	level_print(temp_lev);
	level_free(temp_lev);
}
*/
