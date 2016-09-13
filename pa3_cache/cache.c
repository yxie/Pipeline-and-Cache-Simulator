/*
 * 
 * cache.c
 * 
 * Donald Yeung
 */


#include <stdio.h>
#include <math.h>

#include "cache.h"
#include "main.h"

/* cache configuration parameters */
static int cache_split = 0;
static int cache_usize = DEFAULT_CACHE_SIZE;
static int cache_isize = DEFAULT_CACHE_SIZE; 
static int cache_dsize = DEFAULT_CACHE_SIZE;
static int cache_block_size = DEFAULT_CACHE_BLOCK_SIZE;
static int words_per_block = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
static int cache_assoc = DEFAULT_CACHE_ASSOC;
static int cache_writeback = DEFAULT_CACHE_WRITEBACK;
static int cache_writealloc = DEFAULT_CACHE_WRITEALLOC;

/* cache model data structures */
static Pcache icache;
static Pcache dcache;
static cache c1;
static cache c2;
static cache_stat cache_stat_inst;
static cache_stat cache_stat_data;

/************************************************************/
void set_cache_param(param, value)
  int param;
  int value;
{

  switch (param) {
  case CACHE_PARAM_BLOCK_SIZE:
    cache_block_size = value;
    words_per_block = value / WORD_SIZE;
    break;
  case CACHE_PARAM_USIZE:
    cache_split = FALSE;
    cache_usize = value;
    break;
  case CACHE_PARAM_ISIZE:
    cache_split = TRUE;
    cache_isize = value;
    break;
  case CACHE_PARAM_DSIZE:
    cache_split = TRUE;
    cache_dsize = value;
    break;
  case CACHE_PARAM_ASSOC:
    cache_assoc = value;
    break;
  case CACHE_PARAM_WRITEBACK:
    cache_writeback = TRUE;
    break;
  case CACHE_PARAM_WRITETHROUGH:
    cache_writeback = FALSE;
    break;
  case CACHE_PARAM_WRITEALLOC:
    cache_writealloc = TRUE;
    break;
  case CACHE_PARAM_NOWRITEALLOC:
    cache_writealloc = FALSE;
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
	
  /* initialize the cache, and cache statistics data structures */
	int i;
  /* initialize the cache, and cache statistics data structures */
  //cache size
  if(cache_split == FALSE){
	  c1.size = cache_usize;
	  
	  //associativity
	  c1.associativity = cache_assoc;
	  
	  //set number
	  c1.n_sets = c1.size / (cache_block_size * cache_assoc);
	  
	  //mask & offset
	  c1.index_mask = (c1.n_sets-1) << (LOG2(cache_block_size));
	  c1.index_mask_offset = LOG2(cache_block_size);
	  
	  //LRU
	   c1.LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line)*c1.n_sets);
	   c1.LRU_tail = (Pcache_line *)malloc(sizeof(Pcache_line)*c1.n_sets);
	   
	 for(i=0;i<c1.n_sets;i++){
	  c1.LRU_head[i]=NULL;
	  c1.LRU_tail[i]=NULL;
		}
	   
	   
	   //Set_Contents
	   c1.set_contents = (int *)malloc(sizeof(int)*c1.n_sets);
	   
	   for(i=0;i<c1.n_sets;i++){
		c1.set_contents[i]=0;
	   }
    }
	else{
		c1.size = cache_dsize;
		c2.size = cache_isize;
		
		 //associativity
	  c1.associativity = cache_assoc;
	  c2.associativity = cache_assoc;
	  
	  //set number
	  c1.n_sets = c1.size / (cache_block_size * cache_assoc);
	  c2.n_sets = c2.size / (cache_block_size * cache_assoc);
	  
	  //mask & offset
	  c1.index_mask = (c1.n_sets-1) << (LOG2(cache_block_size));
	  c1.index_mask_offset = LOG2(cache_block_size);
	  
	  c2.index_mask = (c2.n_sets-1) << (LOG2(cache_block_size));
	  c2.index_mask_offset = LOG2(cache_block_size);
	  
	  //LRU
	   c1.LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line)*c1.n_sets);
	   c1.LRU_tail = (Pcache_line *)malloc(sizeof(Pcache_line)*c1.n_sets);
	   
	   c2.LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line)*c2.n_sets);
	   c2.LRU_tail = (Pcache_line *)malloc(sizeof(Pcache_line)*c2.n_sets);
	   
	  for(i=0;i<c1.n_sets;i++){
	  c1.LRU_head[i]=NULL;
	  c1.LRU_tail[i]=NULL;
		}
		
	for(i=0;i<c2.n_sets;i++){
	  c2.LRU_head[i]=NULL;
	  c2.LRU_tail[i]=NULL;
		}
	   
	   
	   //Set_Contents
	   c1.set_contents = (int *)malloc(sizeof(int)*c1.n_sets);
	   c2.set_contents = (int *)malloc(sizeof(int)*c2.n_sets);
	   
	   for(i=0;i<c1.n_sets;i++){
		c1.set_contents[i]=0;
	   }
	   
	   for(i=0;i<c2.n_sets;i++){
		c2.set_contents[i]=0;
	   }
	}
}
/************************************************************/

/************************************************************/
void perform_access(addr, access_type)
  unsigned addr, access_type;
{

  /* handle an access to the cache */
  int index, tag;
  Pcache_line temp_line, temp_line2, head_line;
  int find=0;
  //if(addr==0x44ff80) printf("%X\n", addr);
  
  if(access_type == 0 || access_type ==1){
	cache_stat_data.accesses++;
  }
  else if(access_type == 2)
  {
	cache_stat_inst.accesses++;
  }
  
  /********************************************************/
  if(cache_split == FALSE){ //unified
	  
		index = (addr & c1.index_mask) >> c1.index_mask_offset;
		tag = (addr) >> ( c1.index_mask_offset+LOG2(c1.n_sets) );
	  
  
		if(c1.LRU_head[index]==NULL){ //cache compulsory miss
		//printf("Compulsory Miss\n");
			if(access_type == 0 || access_type == 1)
				cache_stat_data.misses++;
			else if(access_type == 2)
				cache_stat_inst.misses++;
		
			switch(access_type){
			case 0: //read data
				cache_stat_data.demand_fetches += cache_block_size/4;
				temp_line = (Pcache_line *)malloc(sizeof(cache_line));
				temp_line->tag = tag;
				temp_line->LRU_next = NULL;
				temp_line->LRU_prev = NULL;
				//c1.LRU_head[index] = temp_line;
				insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line);
				c1.set_contents[index]++;
			break;	
			case 1:
				if(cache_writealloc ){
					cache_stat_data.demand_fetches += cache_block_size/4;
					temp_line = (Pcache_line *)malloc(sizeof(cache_line));
					temp_line->tag = tag;
					temp_line->LRU_next = NULL;
					temp_line->LRU_prev = NULL;
					//c1.LRU_head[index] = temp_line;	
					insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line);
					if(cache_writeback)
						c1.LRU_head[index]->dirty = 1;
					else
						cache_stat_data.copies_back += 1;
					c1.set_contents[index]++;
				}
				else{
						cache_stat_data.copies_back += 1;
				}
				

			break;
			case 2:
				cache_stat_inst.demand_fetches += cache_block_size/4;
				temp_line = (Pcache_line *)malloc(sizeof(cache_line));
				temp_line->tag = tag;
				temp_line->LRU_next = NULL;
				temp_line->LRU_prev = NULL;
				//c1.LRU_head[index] = temp_line;
				insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line);
				c1.set_contents[index]++;
			break;
			}
		}
		else{  //check hit or not
			head_line = c1.LRU_head[index];
			
			while(head_line != NULL && find == 0){
				if(head_line->tag == tag)
					find = 1;
				else
				 head_line = head_line->LRU_next;
			}
			
			
			
			if(head_line == NULL){ //miss
			//printf("Cache Miss\n");
				if(access_type == 0 || access_type == 1)
					cache_stat_data.misses++;
				else if(access_type == 2)
					cache_stat_inst.misses++;
				
				switch(access_type){
					case 0: //read data
						cache_stat_data.demand_fetches += cache_block_size/4;
						temp_line = (Pcache_line *)malloc(sizeof(cache_line));
						temp_line->tag = tag;
						temp_line->dirty = 0;
						temp_line->LRU_next = NULL;
						temp_line->LRU_prev = NULL;
						if(c1.set_contents[index] == c1.associativity){ //full, delete cache at tail, insert new cache at head
							if(cache_writeback && c1.LRU_tail[index]->dirty == 1)  //writeback data
								cache_stat_data.copies_back += cache_block_size/4;
							delete(&c1.LRU_head[index], &c1.LRU_tail[index], c1.LRU_tail[index]);
							insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line);
							cache_stat_data.replacements++;
							
						}
						else{
							insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line);
							c1.set_contents[index]++;
						}
					break;
					case 1: //write data
						if(cache_writealloc){
							if(cache_writeback == 0)
								cache_stat_data.copies_back += 1;
							cache_stat_data.demand_fetches += cache_block_size/4;
							temp_line = (Pcache_line *)malloc(sizeof(cache_line));
							temp_line->tag = tag;
							if(cache_writeback)
								temp_line->dirty = 1;
							temp_line->LRU_next = NULL;
							temp_line->LRU_prev = NULL;
							//cache_stat_data.replacements++;
							if(c1.set_contents[index] == c1.associativity){//full, replace the tail cache
								if(cache_writeback && c1.LRU_tail[index]->dirty == 1)  //writeback data
									cache_stat_data.copies_back += cache_block_size/4;
								delete(&c1.LRU_head[index], &c1.LRU_tail[index], c1.LRU_tail[index]);
								insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line);
								cache_stat_data.replacements++;
							}
							//c1.LRU_head[index]->tag = tag;
							//c1.LRU_head[index]->dirty = 1;
							else{
								insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line);
								c1.set_contents[index]++;
							}
						}
						else{
							cache_stat_data.copies_back += 1;
						}
					break;
					case 2: //read inst
						cache_stat_inst.demand_fetches += cache_block_size/4;
						temp_line = (Pcache_line *)malloc(sizeof(cache_line));
						temp_line->tag = tag;
						temp_line->dirty = 0;
						temp_line->LRU_next = NULL;
						temp_line->LRU_prev = NULL;
						if(c1.set_contents[index] == c1.associativity){ //full, delete cache at tail, insert new cache at head
							if(cache_writeback && c1.LRU_tail[index]->dirty == 1)  //writeback data
								cache_stat_data.copies_back += cache_block_size/4;
							delete(&c1.LRU_head[index], &c1.LRU_tail[index], c1.LRU_tail[index]);
							insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line);
							cache_stat_inst.replacements++;
						}
						else{
							insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line);
							c1.set_contents[index]++;
						}
					break;
				}
			}
			else{ //cache hit, head_line->tag = tag
			//printf("Cache hit\n");
				if(access_type == 1) //write data
					if(cache_writeback && head_line->dirty == 0)
						head_line->dirty = 1;
					else if(cache_writeback == 0) //write thru
						cache_stat_data.copies_back += 1;
				delete(&c1.LRU_head[index], &c1.LRU_tail[index], head_line);
				insert(&c1.LRU_head[index], &c1.LRU_tail[index], head_line);
			}
		}
  }
  /**********************************************************/
  else{ //split
	if(access_type == 0 || access_type == 1){
		index = (addr & c1.index_mask) >> c1.index_mask_offset;
		tag = (addr) >> ( c1.index_mask_offset+LOG2(c1.n_sets) );
	}
	else{
		index = (addr & c2.index_mask) >> c2.index_mask_offset;
		tag = (addr) >> ( c2.index_mask_offset+LOG2(c2.n_sets) );
	}
  
	if( access_type == 0 || access_type == 1){ 
		if( c1.LRU_head[index]==NULL){ //cache compulsory miss
		//printf("D compulsory miss\n");
			cache_stat_data.misses++;
		
			switch(access_type){
			case 0: //read data
				cache_stat_data.demand_fetches += cache_block_size/4;
				temp_line = (Pcache_line *)malloc(sizeof(cache_line));
				temp_line->tag = tag;
				temp_line->LRU_next = NULL;
				temp_line->LRU_prev = NULL;
				//c1.LRU_head[index] = temp_line;
				insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line);
				c1.set_contents[index]++;
			break;	
			case 1:
				if(cache_writealloc){
					cache_stat_data.demand_fetches += cache_block_size/4;
					temp_line = (Pcache_line *)malloc(sizeof(cache_line));
					temp_line->tag = tag;
					temp_line->LRU_next = NULL;
					temp_line->LRU_prev = NULL;
					//c1.LRU_head[index] = temp_line;	
					insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line);
					if(cache_writeback)
						c1.LRU_head[index]->dirty = 1;
					else{
						cache_stat_data.copies_back += 1;
					}
					c1.set_contents[index]++;
				}	
				else{
					cache_stat_data.copies_back += 1;
				}
			break;
	
			}
		}
		else{  //check hit or not
			head_line = c1.LRU_head[index];
			
			
			while(head_line != NULL && find == 0){
				if(head_line->tag == tag)
					find = 1;
				else
				 head_line = head_line->LRU_next;
			}
			
			
			
			if(head_line == NULL){ //miss
			//printf("D cache miss\n");
				cache_stat_data.misses++;
				
				switch(access_type){
					case 0: //read data
						cache_stat_data.demand_fetches += cache_block_size/4;
						temp_line = (Pcache_line *)malloc(sizeof(cache_line));
						temp_line->tag = tag;
						temp_line->dirty = 0;
						temp_line->LRU_next = NULL;
						temp_line->LRU_prev = NULL;
						if(c1.set_contents[index] == c1.associativity){ //full, delete cache at tail, insert new cache at head
							if(cache_writeback && c1.LRU_tail[index]->dirty == 1)  //writeback data
								cache_stat_data.copies_back += cache_block_size/4;
							temp_line2 = c1.LRU_tail[index];
							delete(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line2);
							insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line);
							cache_stat_data.replacements++;
							
						}
						else{
							insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line);
							c1.set_contents[index]++;
						}
					break;
					case 1: //write data
						if(cache_writealloc){
							if(cache_writeback == 0)
									cache_stat_data.copies_back += 1;
							cache_stat_data.demand_fetches += cache_block_size/4;
							temp_line = (Pcache_line *)malloc(sizeof(cache_line));
							temp_line->tag = tag;
							if(cache_writeback)
								temp_line->dirty = 1;
							temp_line->LRU_next = NULL;
							temp_line->LRU_prev = NULL;
							//cache_stat_data.replacements++;
							if(c1.set_contents[index] == c1.associativity){//full, replace the tail cache
								if(cache_writeback && c1.LRU_tail[index]->dirty == 1)  //writeback data
									cache_stat_data.copies_back += cache_block_size/4;
								temp_line2 = c1.LRU_tail[index];
								delete(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line2);
								insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line);
								cache_stat_data.replacements++;
							}
							//c1.LRU_head[index]->tag = tag;
							//c1.LRU_head[index]->dirty = 1;
							else{
								insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp_line);
								c1.set_contents[index]++;
							}
						}
						else{
							cache_stat_data.copies_back += 1;
						}
					break;
					
				}
			}
			else{ //cache hit, head_line->tag = tag
			//printf("D Cache hit\n");
				if(access_type == 1) //write data
					if(cache_writeback)
						head_line->dirty = 1;
					else if(cache_writeback == 0) //write thru{
						cache_stat_data.copies_back += 1;
				delete(&c1.LRU_head[index], &c1.LRU_tail[index], head_line);
				insert(&c1.LRU_head[index], &c1.LRU_tail[index], head_line);
			}
		}
	}
	
	else if(access_type == 2){ //instr
	
		if( c2.LRU_head[index]==NULL){ //cache compulsory miss
				cache_stat_inst.misses++;
				cache_stat_inst.demand_fetches += cache_block_size/4;
				temp_line = (Pcache_line *)malloc(sizeof(cache_line));
				temp_line->tag = tag;
				temp_line->LRU_next = NULL;
				temp_line->LRU_prev = NULL;
				//c1.LRU_head[index] = temp_line;
				insert(&c2.LRU_head[index], &c2.LRU_tail[index], temp_line);
				c2.set_contents[index]++;		
		}
		else{  //check hit or not
			head_line = c2.LRU_head[index];
			
			find = 0;
			while(head_line != NULL && find == 0){
				if(head_line->tag == tag)
					find = 1;
				else
				 head_line = head_line->LRU_next;
			}
			
			
			
			if(head_line == NULL){ //miss
			//printf("I cache miss\n");
				cache_stat_inst.misses++;
				
						cache_stat_inst.demand_fetches += cache_block_size/4;
						temp_line = (Pcache_line *)malloc(sizeof(cache_line));
						temp_line->tag = tag;
						temp_line->dirty = 0;
						temp_line->LRU_next = NULL;
						temp_line->LRU_prev = NULL;
						if(c2.set_contents[index] == c2.associativity){ //full, delete cache at tail, insert new cache at head
							temp_line2 = c2.LRU_tail[index];
							delete(&c2.LRU_head[index], &c2.LRU_tail[index], temp_line2);
							insert(&c2.LRU_head[index], &c2.LRU_tail[index], temp_line);
							cache_stat_inst.replacements++;
						}
						else{
							insert(&c2.LRU_head[index], &c2.LRU_tail[index], temp_line);
							c2.set_contents[index]++;
						}
			
			}
			
			else{ //cache hit, head_line->tag = tag
			//printf("i cache hit\n");
				delete(&c2.LRU_head[index], &c2.LRU_tail[index], head_line);
				insert(&c2.LRU_head[index], &c2.LRU_tail[index], head_line);
			}
			
		}
	}
  
  }
}
/************************************************************/

/************************************************************/
void flush()
{

  /* flush the cache */
	int i;
	Pcache_line head_line;
	head_line = (Pcache_line *)malloc(sizeof(cache_line));
	
	if(cache_writeback){
		for(i=0;i<c1.n_sets;i++){
			if(c1.LRU_head[i]!=NULL ){
				head_line = c1.LRU_head[i];
				while(head_line != NULL){
					if(head_line->dirty == 1){
						head_line->dirty = 0;
						cache_stat_data.copies_back += cache_block_size/4;
					}
					head_line = head_line->LRU_next;
				}
					
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
  if (cache_split) {
    printf("\tSplit I- D-cache\n");
    printf("\tI-cache size: \t%d\n", cache_isize);
    printf("\tD-cache size: \t%d\n", cache_dsize);
  } else {
    printf("\tUnified I- D-cache\n");
    printf("\tSize: \t%d\n", cache_usize);
  }
  printf("\tAssociativity: \t%d\n", cache_assoc);
  printf("\tBlock size: \t%d\n", cache_block_size);
  printf("\tWrite policy: \t%s\n", 
	 cache_writeback ? "WRITE BACK" : "WRITE THROUGH");
  printf("\tAllocation policy: \t%s\n",
	 cache_writealloc ? "WRITE ALLOCATE" : "WRITE NO ALLOCATE");
}
/************************************************************/

/************************************************************/
void print_stats()
{
  printf("*** CACHE STATISTICS ***\n");
  printf("  INSTRUCTIONS\n");
  printf("  accesses:  %d\n", cache_stat_inst.accesses);
  printf("  misses:    %d\n", cache_stat_inst.misses);
  printf("  miss rate: %f\n", 
	 (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses);
  printf("  replace:   %d\n", cache_stat_inst.replacements);

  printf("  DATA\n");
  printf("  accesses:  %d\n", cache_stat_data.accesses);
  printf("  misses:    %d\n", cache_stat_data.misses);
  printf("  miss rate: %f\n", 
	 (float)cache_stat_data.misses / (float)cache_stat_data.accesses);
  printf("  replace:   %d\n", cache_stat_data.replacements);

  printf("  TRAFFIC (in words)\n");
  printf("  demand fetch:  %d\n", cache_stat_inst.demand_fetches + 
	 cache_stat_data.demand_fetches);
  printf("  copies back:   %d\n", cache_stat_inst.copies_back +
	 cache_stat_data.copies_back);
}
/************************************************************/
