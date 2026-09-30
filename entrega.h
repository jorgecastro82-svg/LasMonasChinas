#pragma once //Es lo mismo que hacer #ifndef #def #endif

typedef struct{     //Por ahorita solo ordenamos por vip, pero podemos agregarle mas
    int priority;   //En caso de necesitar ordenar por vip y otra cosa
}Entrega;           //Tambien pueden agregar otros atributos (entrega,restaurante)

typedef struct{
    int id;//id unica de empleado
    Entrega arr[4];//Maximo numero de entregas que puede cargar un empleado al mismo tiempo
}Empleado;//Empleado provisional, se necesitara introducir coordenadas o algo para
//poder identificar al individio en un espacio del mapa