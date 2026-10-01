#include "merge.h"

//Pila y estructuras necesarias para el mergesort
void pilaInit(Pila *p,int n){
    p->max = n;
    p->top = 0;
    p->arr = (Index *)calloc(n,sizeof(Index));
}

int isPilaVacia(Pila const *p){return p->top == 0;}
int isPilaLlena(Pila const *p){return p->top == p->max;}

void pilaPush(Pila *p,Index i){
    if(isPilaLlena(p)) return;

    p->arr[p->top++] = i;
}

Index pilaPop(Pila *p){
    if(isPilaVacia(p)) return (Index){0};

    return p->arr[--p->top];
}

//Merge y mergesort ya como tal
void merge(Entrega *arr,Entrega *aux,const Index index, int (*cmp)(Entrega const*,Entrega const*)){
    int i,j,k,mid = (index.left + index.right) / 2;
    i = k = index.left;
    j = mid + 1;

    while(i <= mid && j <= index.right) aux[k++] = (cmp(arr + i,arr + j) >= 0) ? arr[i++] : arr[j++];
    //<= por que queremos orden descendiente no ascendiente ya que un vip tiene prioridad mas alta

    while(i <= mid) aux[k++] = arr[i++];
    while(j <= index.right) aux[k++] = arr[j++];

    for(k = index.left;+ k <= index.right;k++) arr[k] = aux[k];

}

void mergesort(Entrega *arr,const int n, int (*cmp)(Entrega const*,Entrega const*)){
    if(n < 2) return;

    Entrega aux[n];
    Pila p;
    pilaInit(&p,n*2);
    pilaPush(&p,(Index){0,n-1,DIVIDE});
    
    while(!isPilaVacia(&p)){
        Index index = pilaPop(&p);
        
        if(index.left >= index.right) continue; //caso base
        
        if(index.fase == DIVIDE){
            int mid = (index.left + index.right) / 2; 
            pilaPush(&p,(Index){index.left,index.right,MERGE});
            pilaPush(&p,(Index){mid + 1,index.right,DIVIDE});
            pilaPush(&p,(Index){index.left,mid,DIVIDE});
        }
        else merge(arr,aux,index,cmp);
    }

    free(p.arr);
}