#pragma once
#include <stdint.h> 
#include <stdlib.h> 

//we do this for the type of uint64_t because the ids of edges and vertex are inside this range
typedef struct{
    float weight;
    uint64_t dest_id;
    uint64_t edge_id;
}Edge;

typedef struct node{
    Edge data;
    struct node *next;    
}Node;

typedef struct{
    Node *head;
    Node *tail;
}LinkedList;

LinkedList initList();

void push_list(LinkedList *l,Edge e);

//1 if empty, 0 if not
int is_empty(LinkedList l);