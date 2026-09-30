#pragma once 
#include "entrega.h"
#include <stdlib.h>

//Pila y estructuras necesarias para el mergesort
typedef struct{
    int left;
    int right;
}Index;

typedef struct{
    int max;
    int top;
    Index arr;
}Pila;

void pilaInit(Pila *p,int n){
    
}

//Mergesort ya como tal
void mergesort(Entrega *arr,const int n);