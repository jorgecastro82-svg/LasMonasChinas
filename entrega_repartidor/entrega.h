#pragma once //Es lo mismo que hacer #ifndef #def #endif
#include <stdlib.h>
#define  MAXENTREGA 4
#define ROJO "\033[0;31m"
#define  BLANCO "\033[0m"
#define BLUE    "\x1b[34m"
#define CYAN    "\033[96m"
typedef enum {LOW = 0,MEDIUM = 1,HIGH = 2} prioridad;

typedef struct{             //Por ahorita solo ordenamos por vip, pero podemos agregarle mas
    int id;
    prioridad priority;     //En caso de necesitar ordenar por vip y otra cosa
}Entrega;                   //Tambien pueden agregar otros atributos (entrega,restaurante)

typedef struct{
    int id;//id unica de empleado
    Entrega arr[MAXENTREGA];//Maximo numero de entregas que puede cargar un empleado al mismo tiempo
}Empleado;//Empleado provisional, se necesitara introducir coordenadas o algo para
//poder identificar al individio en un espacio del mapa

int entrega_cmp(Entrega const*,Entrega const*);

void imprimirEntregas(Entrega const *arr);
char *tipo_prio(int);
Entrega entregaInit(prioridad p);
