#include "entrega.h"

int entrega_cmp(Entrega const *a,Entrega const *b){ //
    return a->priority - b->priority;
}