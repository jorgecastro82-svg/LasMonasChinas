#include "entrega.h"

int entrega_cmp(Entrega const *a,Entrega const *b){ //
    return a->priority - b->priority;
}
void imprimirEntregas(Entrega const *arr){
    for (int i=0;i<MAXENTREGA*MAXENTREGA;i++){
        printf(AZUL"-"BLANCO);
    }
    for (int i=0;i<MAXENTREGA;i++){
        printf(AZUL"|"BLANCO"%d",arr[i].id);
        printf(AZUL" |"BLANCO);
    }
    printf("\n");
    for (int i=0;i<MAXENTREGA*MAXENTREGA;i++){
        printf(AZUL"-"BLANCO);
    }
    for (int i=0;i<MAXENTREGA;i++){
        printf(AZUL"|"BLANCO"%s",tipo_prio(arr[i].priority));
        printf(AZUL" |"BLANCO);
    }
    char *tipo_prio(int n){
        switch(n){
            case 0:
                return "LOW"
            break;
            case 1:
                return "MEDIUM"
            break;
            case 2:
                return "HIGH"
            break;
            default:
                return "-----"
            break;
        
        }
    }
}