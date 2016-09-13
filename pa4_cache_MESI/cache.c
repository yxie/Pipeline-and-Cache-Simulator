#include <stdio.h>
#include <math.h>

#include "cache.h"
#include "main.h"

/* cache configuration parameters */
static int cache_usize = DEFAULT_CACHE_SIZE;
static int cache_block_size = DEFAULT_CACHE_BLOCK_SIZE;
static int words_per_block = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
static int cache_assoc = DEFAULT_CACHE_ASSOC;
static int cache_writeback = DEFAULT_CACHE_WRITEBACK;
static int cache_writealloc = DEFAULT_CACHE_WRITEALLOC;
static int num_core = DEFAULT_NUM_CORE;

/* cache model data structures */
/* max of 8 cores */
static cache mesi_cache[8];
static cache_stat mesi_cache_stat[8];

/************************************************************/
void set_cache_param(param, value)
  int param;
  int value;
{
  switch (param) {
  case NUM_CORE:
    num_core = value;
    break;
  case CACHE_PARAM_BLOCK_SIZE:
    cache_block_size = value;
    words_per_block = value / WORD_SIZE;
    break;
  case CACHE_PARAM_USIZE:
    cache_usize = value;
    break;
  case CACHE_PARAM_ASSOC:
    cache_assoc = value;
    break;
  default:
    printf("error set_cache_param: bad parameter value\n");
    exit(-1);
  }
}
/************************************************************/

/************************************************************/
void init_cache()
{
	
	
	int i,j;
	for(i=0;i<num_core;i++){
		mesi_cache[i].size = cache_usize;
		mesi_cache[i].associativity = cache_assoc;
		mesi_cache[i].n_sets = cache_usize / (cache_block_size * cache_assoc);
		mesi_cache[i].index_mask = (mesi_cache[i].n_sets-1) << (LOG2(cache_block_size));
	    mesi_cache[i].index_mask_offset = LOG2(cache_block_size);
		
		mesi_cache[i].LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line)*mesi_cache[i].n_sets);
	    mesi_cache[i].LRU_tail = (Pcache_line *)malloc(sizeof(Pcache_line)*mesi_cache[i].n_sets);
		
		for(j=0;j<mesi_cache[i].n_sets;j++){
			mesi_cache[i].LRU_head[j]=NULL;
			mesi_cache[i].LRU_tail[j]=NULL;
		}
		
		mesi_cache[i].set_contents = (int *)malloc(sizeof(int)*mesi_cache[i].n_sets);
		
		for(j=0;j<mesi_cache[i].n_sets;j++){
			mesi_cache[i].set_contents[j]=0;
	    }
	}
  /* initialize the cache, and cache statistics data structures */

}
/************************************************************/

/************************************************************/
void perform_access(addr, access_type, pid)
     unsigned addr, access_type, pid;
{
  /* handle accesses to the mesi caches */
  int index, tag;
  Pcache_line temp_line, temp_line2, head_line, head_line2, head_line3;
  int find=0;
  int i,j;
  
	mesi_cache_stat[pid].accesses++;
  
	index = (addr & mesi_cache[pid].index_mask) >> mesi_cache[pid].index_mask_offset;
	tag = (addr) >> ( mesi_cache[pid].index_mask_offset+LOG2(mesi_cache[pid].n_sets) );
	
	if(access_type == 0){
			head_line = (Pcache_line *)malloc(sizeof(cache_line));
			head_line = mesi_cache[pid].LRU_head[index];
			find=0;
			while(head_line != NULL && find == 0){
				if(head_line->tag == tag)
					find = 1;
				else
				 head_line = head_line->LRU_next;
			}
			
			if(head_line == NULL){ //miss
				mesi_cache_stat[pid].misses++;
				mesi_cache_stat[pid].broadcasts++;	//broadcast
				find=0;
				for(i=0;i<num_core && find==0;i++){
					if(i!=pid){
						head_line2 = (Pcache_line *)malloc(sizeof(cache_line));
						head_line2 = mesi_cache[i].LRU_head[index];
						while(head_line2 != NULL && find == 0){
							if(head_line2->tag == tag)
								find = 1;
							else
								head_line2 = head_line2->LRU_next;
						}
					}
				}
				if(find == 1){ //found in other caches
					if(head_line2->state == EXCLUSIVE || head_line2->state == MODIFIED){
							if(head_line2->state == MODIFIED)
								mesi_cache_stat[pid].copies_back += cache_block_size/4; //pay attention to this case
							mesi_cache_stat[pid].demand_fetches += cache_block_size/4;	
							head_line2->state = SHARED;
							
							temp_line = (Pcache_line *)malloc(sizeof(cache_line));
							temp_line->state = SHARED;	
							temp_line->tag = tag;
							temp_line->LRU_next = NULL;
							temp_line->LRU_prev = NULL;
							if(mesi_cache[pid].set_contents[index] == mesi_cache[pid].associativity){ //full, delete cache at tail, insert new cache at head
								if(mesi_cache[pid].LRU_tail[index]->state == MODIFIED)  //writeback data
									mesi_cache_stat[pid].copies_back += cache_block_size/4;
								temp_line2 = (Pcache_line *)malloc(sizeof(cache_line));
								temp_line2 = mesi_cache[pid].LRU_tail[index];
								delete(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], temp_line2);
								insert(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], temp_line);
								mesi_cache_stat[pid].replacements++;
							}
							else{
								insert(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], temp_line);
								mesi_cache[pid].set_contents[index]++;
							}
					}
					else if(head_line2->state == SHARED){
							mesi_cache_stat[pid].demand_fetches += cache_block_size/4; //read from other caches
							
							temp_line = (Pcache_line *)malloc(sizeof(cache_line));
							temp_line->state = SHARED;	
							temp_line->tag = tag;
							temp_line->LRU_next = NULL;
							temp_line->LRU_prev = NULL;
							if(mesi_cache[pid].set_contents[index] == mesi_cache[pid].associativity){ //full, delete cache at tail, insert new cache at head
								if(mesi_cache[pid].LRU_tail[index]->state == MODIFIED)  //writeback data
									mesi_cache_stat[pid].copies_back += cache_block_size/4;
								temp_line2 = (Pcache_line *)malloc(sizeof(cache_line));
								temp_line2 = mesi_cache[pid].LRU_tail[index];
								delete(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], temp_line2);
								insert(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], temp_line);
								mesi_cache_stat[pid].replacements++;
							}
							else{
								insert(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], temp_line);
								mesi_cache[pid].set_contents[index]++;
							}
					}
				}
				else{	//can't find in other caches
					mesi_cache_stat[pid].demand_fetches += cache_block_size/4; //read from memory
					temp_line = (Pcache_line *)malloc(sizeof(cache_line));
					temp_line->state = EXCLUSIVE;
					temp_line->tag = tag;
					temp_line->LRU_next = NULL;
					temp_line->LRU_prev = NULL;
					if(mesi_cache[pid].set_contents[index] == mesi_cache[pid].associativity){ //full, delete cache at tail, insert new cache at head
						if(mesi_cache[pid].LRU_tail[index]->state == MODIFIED)  //writeback data
							mesi_cache_stat[pid].copies_back += cache_block_size/4;
						temp_line2 = (Pcache_line *)malloc(sizeof(cache_line));
						temp_line2 = mesi_cache[pid].LRU_tail[index];
						delete(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], temp_line2);
						insert(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], temp_line);
						mesi_cache_stat[pid].replacements++;
					}
					else{
						insert(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], temp_line);
						mesi_cache[pid].set_contents[index]++;
					}
				}
			}
			else{ //cache hit, head_line->tag = tag
				delete(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], head_line);
				insert(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], head_line);
			}
	} //end if(access_type == 0)
	else if(access_type == 1){
		head_line = (Pcache_line *)malloc(sizeof(cache_line));
		head_line = mesi_cache[pid].LRU_head[index];
		find=0;
		while(head_line != NULL && find == 0){
			if(head_line->tag == tag)
				find = 1;
			else
				head_line = head_line->LRU_next;
		}
		if(head_line == NULL){ //miss
			mesi_cache_stat[pid].misses++;
			mesi_cache_stat[pid].broadcasts++;	//broadcast
			find=0;
			for(i=0;i<num_core && find==0;i++){
				head_line2 = (Pcache_line *)malloc(sizeof(cache_line));
				head_line2 = mesi_cache[i].LRU_head[index];
				while(head_line2 != NULL && find == 0){
					if(head_line2->tag == tag)
						find = 1;
					else
						head_line2 = head_line2->LRU_next;
				}
			}
			if(find == 1){ //found in other caches
				mesi_cache_stat[pid].demand_fetches += cache_block_size/4;	
				for(i=0;i<num_core;i++){
					if(i!=pid){
						head_line2 = (Pcache_line *)malloc(sizeof(cache_line));
						head_line2 = mesi_cache[i].LRU_head[index];
						while(head_line2 != NULL){
							if(head_line2->tag == tag){
								//head_line2->state = INVALID; //state of other caches become invalid
								delete(&mesi_cache[i].LRU_head[index], &mesi_cache[i].LRU_tail[index], head_line2);
								head_line2=NULL;
								mesi_cache[i].set_contents[index]--;
							}
							else
								head_line2 = head_line2->LRU_next;
						}
					}
				}
						
				temp_line = (Pcache_line *)malloc(sizeof(cache_line));
				temp_line->state = MODIFIED;	//state of local cache change from INVALID to MODIFIED
				temp_line->tag = tag;
				temp_line->LRU_next = NULL;
				temp_line->LRU_prev = NULL;
				if(mesi_cache[pid].set_contents[index] == mesi_cache[pid].associativity){ //full, delete cache at tail, insert new cache at head
					if(mesi_cache[pid].LRU_tail[index]->state == MODIFIED)  //writeback data
						mesi_cache_stat[pid].copies_back += cache_block_size/4;
					temp_line2 = (Pcache_line *)malloc(sizeof(cache_line));
					temp_line2 = mesi_cache[pid].LRU_tail[index];
					delete(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], temp_line2);
					insert(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], temp_line);
					mesi_cache_stat[pid].replacements++;
				}
				else{
					insert(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], temp_line);
					mesi_cache[pid].set_contents[index]++;
				}
					
			}
			else{	//can't find in other caches
				mesi_cache_stat[pid].demand_fetches += cache_block_size/4; //read from memory
				temp_line = (Pcache_line *)malloc(sizeof(cache_line));
				temp_line->state = MODIFIED;
				temp_line->tag = tag;
				temp_line->LRU_next = NULL;
				temp_line->LRU_prev = NULL;
				if(mesi_cache[pid].set_contents[index] == mesi_cache[pid].associativity){ //full, delete cache at tail, insert new cache at head
					if(mesi_cache[pid].LRU_tail[index]->state == MODIFIED)  //writeback data
						mesi_cache_stat[pid].copies_back += cache_block_size/4;
					temp_line2 = (Pcache_line *)malloc(sizeof(cache_line));
					temp_line2 = mesi_cache[pid].LRU_tail[index];
					delete(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], temp_line2);
					insert(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], temp_line);
					mesi_cache_stat[pid].replacements++;
				}
				else{
					insert(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], temp_line);
					mesi_cache[pid].set_contents[index]++;
				}
			}
		}
		else{ //cache hit, head_line->tag = tag
			if(head_line->state == EXCLUSIVE){
				head_line->state = MODIFIED;
			}
			else if(head_line->state == SHARED){
				head_line->state = MODIFIED;
				mesi_cache_stat[pid].broadcasts++;
				for(i=0;i<num_core;i++){
					if(i!=pid){
						head_line2 = (Pcache_line *)malloc(sizeof(cache_line));
						head_line2 = mesi_cache[i].LRU_head[index];
						while(head_line2 != NULL){
							if(head_line2->tag == tag){
								//head_line2->state = INVALID; //state of other caches become invalid
								delete(&mesi_cache[i].LRU_head[index], &mesi_cache[i].LRU_tail[index], head_line2);
								head_line2=NULL;
								mesi_cache[i].set_contents[index]--;
							}
							else
								head_line2 = head_line2->LRU_next;
						}
					}
				}
			}
			delete(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], head_line);
			insert(&mesi_cache[pid].LRU_head[index], &mesi_cache[pid].LRU_tail[index], head_line);
		}
	}
	
}
/************************************************************/

/************************************************************/
void flush()
{
  /* flush the mesi caches */
  	int i,j;
	Pcache_line head_line;
	head_line = (Pcache_line *)malloc(sizeof(cache_line));
	
	for(j=0;j<num_core;j++){
		for(i=0;i<mesi_cache[j].n_sets;i++){
			
				head_line = mesi_cache[j].LRU_head[i];
				while(head_line != NULL){
					if(head_line->state == MODIFIED){
						head_line->state == INVALID;
						mesi_cache_stat[j].copies_back += cache_block_size/4;
					}
					head_line = head_line->LRU_next;
				}
		
		}
	}
}
/************************************************************/

/************************************************************/
void delete(head, tail, item)
  Pcache_line *head, *tail;
  Pcache_line item;
{
  if (item->LRU_prev) {
    item->LRU_prev->LRU_next = item->LRU_next;
  } else {
    /* item at head */
    *head = item->LRU_next;
  }

  if (item->LRU_next) {
    item->LRU_next->LRU_prev = item->LRU_prev;
  } else {
    /* item at tail */
    *tail = item->LRU_prev;
  }
}
/************************************************************/

/************************************************************/
/* inserts at the head of the list */
void insert(head, tail, item)
  Pcache_line *head, *tail;
  Pcache_line item;
{
  item->LRU_next = *head;
  item->LRU_prev = (Pcache_line)NULL;

  if (item->LRU_next)
    item->LRU_next->LRU_prev = item;
  else
    *tail = item;

  *head = item;
}
/************************************************************/

/************************************************************/
void dump_settings()
{
  printf("Cache Settings:\n");
  printf("\tSize: \t%d\n", cache_usize);
  printf("\tAssociativity: \t%d\n", cache_assoc);
  printf("\tBlock size: \t%d\n", cache_block_size);
}
/************************************************************/

/************************************************************/
void print_stats()
{
  int i;
  int demand_fetches = 0;
  int copies_back = 0;
  int broadcasts = 0;

  printf("*** CACHE STATISTICS ***\n");

  for (i = 0; i < num_core; i++) {
    printf("  CORE %d\n", i);
    printf("  accesses:  %d\n", mesi_cache_stat[i].accesses);
    printf("  misses:    %d\n", mesi_cache_stat[i].misses);
    printf("  miss rate: %f (%f)\n", 
	   (float)mesi_cache_stat[i].misses / (float)mesi_cache_stat[i].accesses,
	   1.0 - (float)mesi_cache_stat[i].misses / (float)mesi_cache_stat[i].accesses);
    printf("  replace:   %d\n", mesi_cache_stat[i].replacements);
  }

  printf("\n");
  printf("  TRAFFIC\n");
  for (i = 0; i < num_core; i++) {
    demand_fetches += mesi_cache_stat[i].demand_fetches;
    copies_back += mesi_cache_stat[i].copies_back;
    broadcasts += mesi_cache_stat[i].broadcasts;
  }
  printf("  demand fetch (words): %d\n", demand_fetches);
  /* number of broadcasts */
  printf("  broadcasts:           %d\n", broadcasts);
  printf("  copies back (words):  %d\n", copies_back);
}
/************************************************************/
