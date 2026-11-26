#include "stringy_buffer.h"

Meta *get_meta(void *list){
    Meta *meta = (void*)((unsigned char*)list) - sizeof(Meta);
    return meta; 
}

int len(void *list){
    return get_meta(list)->size;
}