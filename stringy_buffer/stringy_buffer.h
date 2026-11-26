#pragma once
#include <stdlib.h>
#include <stdint.h>

typedef struct{
    _Alignas(uint64_t) int size;
    int capacity;
    int element_size;
}Meta;

Meta *get_meta(void *list);

int len(void *list);

#define push(LIST,ELEMENT) do{ \
    if(!LIST){ \
        LIST = malloc(sizeof(ELEMENT) * 10 + sizeof(Meta)); \
        LIST = (void*)(((unsigned char*)LIST) + sizeof(Meta)); \
        Meta *meta = get_meta(LIST); \
        meta->size = 0; \
        meta->capacity = 10; \
        meta->element_size = sizeof(ELEMENT); \
    } \
    Meta *meta = get_meta(LIST); \
 \
    if(meta->size == meta->capacity){ \
        /*Starting point is meta because list is actually [meta + elements]*/ \
        void *temp = realloc(meta,meta->capacity * 2 * meta->element_size + sizeof(Meta)); \
        if(!temp) break; \
 \
        LIST = (void*)(((unsigned char*)temp) + sizeof(Meta)); \
        meta = get_meta(LIST); \
        meta->capacity *= 2; \
    } \
 \
    LIST[meta->size++] = ELEMENT; \
}while(0)

//idk
