#include <stdio.h>
#include "merge.h"
#include "entrega.h"
#include <stdlib.h>
#include <time.h>

int main(){
    srand(time(NULL));  //semilla para randoms

    Entrega entregas[10]; //10 para ejemplo pero 4 max por repartidor
    for(int i = 0;i < 10;++i) entregas[i] = entregaInit(rand() % 3);
    imprimirEntregas(entregas);
    printf("\n"); //salto linea
    mergesort(entregas,10,entrega_cmp);
    imprimirEntregas(entregas);

    return 0;
}
