#include "stringy_buffer.h"

Meta *get_meta(void *list){
    Meta *meta = (Meta*)list - 1;
    return meta; 
}

int len(void *list){
    return list ? get_meta(list)->size : 0;
}