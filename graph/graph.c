#include "graph.h"

Graph initGraph(){return (Graph){initHash()};}

void add_vertex(Graph *g,uint64_t vertex){
	insert_key(&g->vertex,vertex);
}

void add_edge(Graph *g,uint64_t src,Edge e){
	LinkedList *neighbors = get_neighbors(g,src);
	if(neighbors == NULL) return;

	push_list(neighbors,e);
}

LinkedList *get_neighbors(Graph *g,uint64_t src){
	Pair *pair = get_val_hash_table(&g->vertex,src);
	if(pair == NULL) return NULL;

	return &pair->neighbors;
}

void print_graph(Graph *g){
    HashTable *h = &(g->vertex);
    Pair *table = h->table;
    for(int i = 0; i < h->capacity;i++){
        if(table[i].vertex == EMPTY) continue;
        printf("\n[%" PRIu64 "] -> ",table[i].vertex);
        if(!is_empty(table[i].neighbors)){
            for(Node *aux = table[i].neighbors.head; aux != NULL; aux = aux->next){
                printf("(%" PRIu64 ",%f) -> ",aux->data.dest_id, aux->data.weight);
            }
        }
        printf("END");
    }
    printf("\n");
}