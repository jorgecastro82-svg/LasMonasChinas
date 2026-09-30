#pragma once 
#include "entrega.h"
#include <stdlib.h>

//Pila y estructuras necesarias para el mergesort
typedef enum {DIVIDE,MERGE}Fase;

typedef struct{
    int left;
    int right;
    Fase fase;
}Index;

typedef struct{
    int max;
    int top;
    Index *arr;
}Pila;

void pilaInit(Pila *p,int n);
int isPilaVacia(Pila const *p);
int isPilaLlena(Pila const *p);

void pilaPush(Pila *p,Index i);

Index pilaPop(Pila *p);

//Merge y mergesort ya como tal
void merge(Entrega *arr,Entrega *aux,const Index index, int (*cmp)(Entrega const*,Entrega const*));

void mergesort(Entrega *arr,const int n, int (*cmp)(Entrega const*,Entrega const*));