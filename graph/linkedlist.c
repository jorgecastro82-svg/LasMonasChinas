#include "linkedlist.h"

LinkedList initList(){return (LinkedList){NULL,NULL};}

void push_list(LinkedList *l,Edge e){
    Node *new_node = malloc(sizeof(Node));
    if(!new_node) return;

    new_node->data = e;
    new_node->next = NULL;
    
    if(l->head == NULL){
        l->head = l->tail = new_node;    
    }
    //to avoid parallel edges we just substitute with the one that has the smaller weight
    else{
        //seach if it exists an edge with the same src/dest but different weight
        for(Node *aux = l->head; aux != NULL; aux = aux->next){
            if(aux->data.dest_id == e.dest_id){
                if(aux->data.weight > e.weight){
                    aux->data = e;
                }
                free(new_node);
                return;
            }
        }
        //if it got here then theres no parallel edge
        l->tail->next = new_node;
        l->tail = new_node;
    }
}

//1 if empty, 0 if not
int is_empty(LinkedList l){return l.head == NULL;}