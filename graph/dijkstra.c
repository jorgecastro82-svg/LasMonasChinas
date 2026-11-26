#include "dijkstra.h"

Heap initHeap(void){return (Heap){NULL};}

void swap_edge(_Dijkstra_edge *a,_Dijkstra_edge *b){
	_Dijkstra_edge temp = *a;
	*a = *b;
	*b = temp;
}

int heap_is_empty(Heap *h){return len(h->data) == 0;}

int heap_push(Heap *h,_Dijkstra_edge value){
	push(h->data,value);

	int i = len(h->data) - 1;
	//siftup
	while(i > 0){
		int parent = (i - 1) / 2;
		if(h->data[parent].weight <= h->data[i].weight) break;
		swap_edge(&h->data[parent],&h->data[i]);
		i = parent;
	}
	return 1;
}

_Dijkstra_edge heap_pop(Heap *h){
	int size = len(h->data);

	_Dijkstra_edge result = h->data[0];

	h->data[0] = h->data[size - 1];
	get_meta(h->data)->size--;
	size--;

	//siftdown
	int i = 0;
	while(1){
		int left = 2*i + 1;
		int right = 2*i + 2;
		int smallest = i;

		if(left < size &&h->data[left].weight < h->data[smallest].weight) smallest = left;
		if(right < size && h->data[right].weight < h->data[smallest].weight) smallest = right;
		if(smallest == i) break;

		swap_edge(&h->data[i],&h->data[smallest]);
		i = smallest;
	}
	return result;
}

_Dijkstra_edge *dijkstra(Graph *g,uint64_t src){
	_Dijkstra_edge *fixed = NULL;
	HashTable *h = &g->vertex;

	//check src exists
	if(get_val_hash_table(h,src) == NULL) return NULL;

	//Check vertexes visited 0 for false, 1 for true, unsigned char to not waste bytes like in int
    //with pointer substraction well get the index to avoid doing searches in the visited array
	unsigned char *visited = calloc(h->capacity,sizeof(unsigned char));
	if(!visited) return NULL; //case failed

	Heap heap = initHeap();

	//initial edge should be of the type (src,src,0)
	heap_push(&heap,(_Dijkstra_edge){src,src,0,0});

	while(!heap_is_empty(&heap)){
		_Dijkstra_edge current = heap_pop(&heap);

		Pair *pair = get_val_hash_table(h,current.dest_id);
		if(pair == NULL) continue;

        //cool trick, because h->table is the initial position, and pair is h->table + some offset
        //we get the index by doing substraction
		int index = pair - h->table; 
		if(visited[index]) continue;

		visited[index] = 1; //if not visited then now it is

		//NEVER ADD SRC
		if(current.dest_id != src) push(fixed,current);
        
        //add all neighbors to the heap
		LinkedList *neighbors = get_neighbors(g,current.dest_id);
		for(Node *aux = neighbors->head; aux != NULL; aux = aux->next){
			heap_push(&heap,(_Dijkstra_edge){current.dest_id,aux->data.dest_id,aux->data.edge_id,current.weight + aux->data.weight});
		}
	}

	free(visited);
	if(heap.data) free(get_meta(heap.data));
	return fixed;
}