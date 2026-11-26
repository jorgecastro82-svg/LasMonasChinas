#pragma once
#include "graph.h"
#include "../stringy_buffer/stringy_buffer.h"

typedef struct{
    uint64_t src_id,dest_id,edge_id;
    float weight;
}_Dijkstra_edge; //This edge is only used for simplification of dijkstra algorithm

typedef struct{
	_Dijkstra_edge *data;
}Heap;

Heap initHeap(void);
int heap_push(Heap *h,_Dijkstra_edge value);
_Dijkstra_edge heap_pop(Heap *h);
int heap_is_empty(Heap *h);
void swap_edge(_Dijkstra_edge *a,_Dijkstra_edge *b);
_Dijkstra_edge *dijkstra(Graph *g,uint64_t source);