#include "entrega.h"

int entrega_cmp(Entrega const *a,Entrega const *b){ //
    return a->priority - b->priority;
}

Entrega entregaInit(prioridad p){
    static int id = 0;
    return (Entrega){id++,p};
}