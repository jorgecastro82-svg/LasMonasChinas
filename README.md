# LasMonasChinas
Proyecto de Analisis de algoritmos

Division
C solo lee de la hoja de vertices sus ids, y de las aristas solo lee (src,dest,weight)
Al hacer dijkstra se obtendra una lista de aristas de la forma (src,dest,weight)
Se convertira cada arista a su id correspondiente y se enviara a un txt

Python leera este txt, y en base a la informacion completa de las aristas y vertices,
Hara plot de el mapa de tijuana, y marcara 

COMPILA CON
gcc main.c graph/*.c stringy_buffer/*.c -o main -lm

EJECTURAR CON
./main src_lat src_lon dest_lat dest_lon
lat y long las sacas de google maps, y nuestro programa.c buscara el nodo mas cercano a esta