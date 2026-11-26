#include "hashtable.h"

//We used SplitMix algorithm to generate hashed_keys, we use it distributes uint64_t values pretty well
uint64_t fold(uint64_t key){
	key += 0x9e3779b97f4a7c15ULL;
	key = (key ^ (key >> 30)) * 0xbf58476d1ce4e5b9ULL;
	key = (key ^ (key >> 27)) * 0x94d049bb133111ebULL;
	key ^= key >> 31;
	return key;
}
//Powers of 2 because of triangular numbers instead of quadratic sounding
HashTable initHash(){return (HashTable){0,32,(Pair*)calloc(32,sizeof(Pair))};}

//Because i doubt well ever have a vertex's id be 0 we use 0 for EMPTY
//ive just realized for our project we never delete nodes or vertexes... so we dont have a DELETED constant
//Because of calloc empty is the default state
void insert_key(HashTable *h,uint64_t key){
    if(((float)h->size) / h->capacity >= 0.70){
        if(!rehash(h)) return; //case failed
    }

    uint64_t base = fold(key) % h->capacity;
    uint64_t index = base;
    uint64_t k = 0; //For quadratic sounding
    while(h->table[index].vertex != EMPTY){
        //Check that the current index doesnt have the key
        if(h->table[index].vertex == key) return;
        ++k;
        index = (base + (k * (k + 1)) / 2) % h->capacity;
    } 

    h->table[index].vertex = key;
    h->size++;
}

void insert_key_val_pair(HashTable *h,uint64_t key,LinkedList value){
    if(((float)h->size) / h->capacity >= 0.49){
        if(!rehash(h)) return; //case failed
    }

    uint64_t base = fold(key) % h->capacity;
    uint64_t index = base;
    uint64_t k = 0; //For quadratic sounding
    while(h->table[index].vertex != EMPTY){
        //Check that the current index doesnt have the key
        if(h->table[index].vertex == key) return;
        ++k;
        index = (base + (k * (k + 1)) / 2) % h->capacity;
    } 

    h->table[index].vertex = key;
    h->table[index].neighbors = value;
    h->size++;
}

Pair *get_val_hash_table(HashTable *h,uint64_t key){
    uint64_t base = fold(key) % h->capacity;
    uint64_t index = base;
    uint64_t k = 0; //For quadratic sounding
    while(h->table[index].vertex != EMPTY){
        if(h->table[index].vertex == key) return &(h->table[index]);
        ++k;
        index = (base + (k * (k + 1)) / 2) % h->capacity;
    } 
    //case not found
    return NULL;
}

//1 if success, 0 if failed
int rehash(HashTable *h){
    Pair *new_table = (Pair*)calloc(h->capacity * 2,sizeof(Pair));
    if(!new_table) return 0;

    HashTable new_hash_table = {0,h->capacity*2,new_table};

    for(int i = 0; i < h->capacity;i++){
        Pair current = h->table[i]; 
        if(current.vertex != EMPTY) insert_key_val_pair(&new_hash_table,current.vertex,current.neighbors);
    }
    free(h->table);
    *h = new_hash_table;
    return 1;
}