#pragma once
#include "graph/dijkstra.h"

int load_graph(Graph *g);
int ruta_inversa(_Dijkstra_edge *fixed, uint64_t source, uint64_t destination,
                 _Dijkstra_edge **out);
int ejecutar_plan(int argc, char **argv);
