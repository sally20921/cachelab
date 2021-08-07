#include "cachelab.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

//we recommend that you use the getopt function to parse your command line arguments. 
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
/*
set = [line0, line2, ..., lineE]
*/

/*line struct
     31                           s+b                  b               0
     |             tag             |    set index      |    offset     |
*/

typedef struct {
  int tag;
  bool valid;
  int time; //this is used for finding LRU (least recently used) line 
} line_t;
//if cache line is valid and tag matches, then cache hit

/*set struct*/
typedef struct {
  line_t *lines;
} set_t;

/*cache struct*/
typedef struct {
  set_t *sets;
  size_t set_num;  
  size_t line_num; // number of lines per set (E)
} cache_t;

/*cache initialization*/
cache_t cache = {};

//     31                           s+b                  b               0
//     |             tag             |    set index      |    offset     |
int set_index_bits = 0; //= s
int block_bits = 0; // = b

/*output*/
size_t hit_count = 0;
size_t miss_count = 0;
size_t eviction_count = 0;

/*replay the given trace file, count number of hits, misses and evictions*/
void simulate(int addr);
void update_cache(set_t *set, size_t line_no);
//whether it is cache hit, cache miss, or eviction
int main(int argc, char *argv[]) {

  FILE *file = 0;

/*1) parse command line arguments*/
//we recommend that you use getopt function to parse your command line arguments
//-s <s> -E <E> -b <b> -t <t> (s = number of set index bits,
//E = number of lines per set, b = number of block bits)
  for (int opt; (opt = getopt(argc, argv, "s:E:b:t:")) != -1;) {
    switch (opt) {
      /*2)compute s, E, b from command line arguments*/
      case 's':
        set_index_bits = atoi(optarg); 
        cache.set_num = 2 << set_index_bits; 
        break;

      case 'E': //number of line per set
        cache.line_num = atoi(optarg); 
        break;

      case 'b':
        block_bits = atoi(optarg);
        break;

      case 't': // Input filename
        if (!(file = fopen(optarg, "r"))) { 
		return 1; 
	}
        break;

      default://otherwise, it is an unknown option
        return 1;
    } //end of switch 

  } //end of for

  if ((!set_index_bits) || (!cache.line_num) || 
	(!block_bits) || (!file)) { 
	return 1; 
  } //exceptional cases

  /*3) initialize your cache simulator*/
  cache.sets = malloc(sizeof(set_t) * cache.set_num);
  for (int i = 0; i < cache.set_num; i++) {
    cache.sets[i].lines = calloc(sizeof(line_t), cache.line_num);
  } 

  //////////////////////////////////////////////////////////////////
  //the format of each memory access is : [space]operation address,size
  char operation;
  int addr;

  while (fscanf(file, " %c %x%*c%*d", &operation, &addr) != EOF) {
   //the address field specifies a 64-bit hexadecimal memory access
    if (operation == 'I') { 
	continue; //we are only interested in data cache performance
		  //ignore all instruction cache accesses 
    }

    simulate(addr);
    //each data load or store can cause at most one cache miss, the data modify operation is treated as a load followed by a store to the same address.
    //Thus, an M operation can result in two cache hits, or a miss and a hit plus a possible eviction.
    if ('M' == operation) { 
	simulate(addr); 
    } 
  }

   fclose(file);
   /*5) free allocated memory*/
   for (int i = 0; i < cache.set_num; i++) { 
	free(cache.sets[i].lines); 
   }
  free(cache.sets);

   /* 6) output the hit and miss statistics for the autographer*/
  /*to receive credit for Part 1, you must all the function printSummary,
    with the total number of hit_count, miss_count, and eviction_count, at 
    the end of main function*/
  printSummary(hit_count, miss_count, eviction_count);

  return 0;
}

/* 4) replays the given trace file against your cache simulator 
      and count the number of hits, misses, and evictions*/
void simulate(int addr) {
  //the address field specifies a 64-bit hexadecimal memory access
  // Get set index and tag bits from the address
  //     31                           s+b                  b               0
  //     |             tag             |    set index      |    offset     |

  int tag = ((addr >> (set_index_bits + block_bits)) & 0xffffffff);
  int set_index = ((addr>>block_bits)&(0x7fffffff >> (31-set_index_bits)));

  //select set for set[set_index]
  set_t *set = &cache.sets[set_index];

  /*check if cache hit*/
  for (int i = 0; i < cache.line_num; i++) {
    line_t* line = &set->lines[i];

    // Check if the cache line is valid
    if (!(line->valid)) { 
	continue; 
    }
    // Compare tag bits
    if ((line->tag) != tag) { 
	continue; 
    }

    /*cache hit*/
    hit_count++;
    update_cache(set, i); // i is the number in lines
    return;
  }
/**********************/

  /*it is cache miss*/
  miss_count++;

  /*check for cache emtpy line*/
  for (int i = 0; i < cache.line_num; i++) {
    line_t* line = &set->lines[i];

    if (line->valid) { 
	continue; 
    }

    line->valid = true;
    line->tag = tag;
    update_cache(set, i); //i is the number in lines 
    return;
  }
  /****************************/

  /*if it is neither cache hit or miss, we need to evict*/
  eviction_count++;

  /*look for least recently used cache line*/
  for (int i = 0; i < cache.line_num; i++) {
    line_t* line = &set->lines[i];

    if (line->time) { 
	continue; //if time is not zero, it is not LRU! move on!
    }

    line->valid = true;
    line->tag = tag;
    update_cache(set, i);
    return;
  }
  /********************************************/
}

/*update_cache based on line number*/
void update_cache(set_t *set, size_t line_number) {

/*typedef struct {
  int tag;
  bool valid;
  int time; //this is used for finding LRU (least recently used) line 
} line_t;*/

  line_t *line = &set->lines[line_number];

  for (int i = 0; i < cache.line_num; i++) {
    line_t *temp = &set->lines[i];
    if (!(temp->valid)) { 
	continue; 
    }
    if ((temp->time) <= (line->time)) { 
	continue; 
    }

    --(temp->time); //time: [0, E)
  }

  (line->time) = (cache.line_num - 1); //set time to recently used 
}
