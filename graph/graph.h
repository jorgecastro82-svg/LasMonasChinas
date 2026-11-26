#pragma once
#include "hashtable.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

typedef struct{
    HashTable vertex;
}Graph; //Adjacency list graph, its directed in this case

Graph initGraph();

void add_edge(Graph *g,uint64_t src,Edge e);

void add_vertex(Graph *g,uint64_t vertex);

LinkedList *get_neighbors(Graph *g,uint64_t src);

void print_graph(Graph *g);