#pragma once
#include <stdint.h>
#include <stdlib.h> 
#include "linkedlist.h"

#define EMPTY 0

typedef struct{
    uint64_t vertex;
    LinkedList neighbors;
}Pair;

typedef struct{
    int size,capacity;
    Pair *table;
}HashTable;

//We used SplitMix algorithm to generate hashed_keys, we use it distributes uint64_t values pretty well
uint64_t fold(uint64_t key);
//Powers of 2 because of triangular numbers instead of quadratic sounding
HashTable initHash();

//Because i doubt well ever have a vertex's id be 0 we use 0 for EMPTY
//ive just realized for our project we never delete nodes or vertexes... so we dont have a DELETED constant
//Because of calloc empty is the default state
void insert_key(HashTable *h,uint64_t key);

void insert_key_val_pair(HashTable *h,uint64_t key,LinkedList value);

Pair *get_val_hash_table(HashTable *h,uint64_t key);

//1 if success, 0 if failed
int rehash(HashTable *h);