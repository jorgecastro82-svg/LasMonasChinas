#include "planificador.h"
#include "merge.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <string.h>

#define PARADAS (MAXENTREGA + 1)
#define METROS_MINUTO 500.0
#define TARIFA 100

typedef struct {
    Entrega entregas[MAXENTREGA];
    uint64_t nodos[PARADAS]; // 0 es el deposito; i corresponde a entregas[i-1].
    _Dijkstra_edge *arbol[PARADAS];
    double dist[PARADAS][PARADAS], salida_min[PARADAS], entrada_min[PARADAS];
    int n, inicio, flexible, orden[MAXENTREGA], mejor_orden[MAXENTREGA];
    double mejor;
} Plan;

static int hora(const char *s){
    if(strlen(s) != 5 || s[2] != ':' || strspn(s, "0123456789") != 2 ||
       strspn(s+3, "0123456789") != 2) return -1;
    int h = atoi(s), m = atoi(s+3);
    return h < 24 && m < 60 ? h*60 + m : -1;
}

static int nodo(const char *s, uint64_t *id){
    if(!*s || s[strspn(s, "0123456789")]) return 0;
    errno = 0;
    *id = strtoull(s, NULL, 10);
    return !errno && *id != 0;
}

static int leer(Plan *p, FILE *f){
    char linea[256];
    if(!fgets(linea, sizeof linea, f)) return 0;
    linea[strcspn(linea, "\r\n")] = 0;
    if(strcmp(linea, "nodo,prioridad,desde,hasta")) return 0;
    while(fgets(linea, sizeof linea, f)){
        linea[strcspn(linea, "\r\n")] = 0;
        char id[22], pr[2], desde[6], hasta[6], extra;
        if(p->n == MAXENTREGA ||
           sscanf(linea, "%21[0-9],%1[012],%5[0-9:],%5[0-9:]%c",
                  id, pr, desde, hasta, &extra) != 4) return 0;
        Entrega e = entregaInit((prioridad)(pr[0]-'0'));
        e.apertura = hora(desde);
        e.cierre = hora(hasta);
        if(!nodo(id, &e.nodo) || e.apertura < 0 || e.cierre < e.apertura) return 0;
        p->entregas[p->n++] = e;
    }
    return !ferror(f) && p->n > 0;
}

static int matriz(Plan *p, Graph *g){
    for(int i=0; i<=p->n; ++i){
        if(!get_val_hash_table(&g->vertex, p->nodos[i])) return 0;
        p->arbol[i] = dijkstra(g, p->nodos[i]);
        for(int j=0; j<=p->n; ++j){
            double d = INFINITY;
            if(p->nodos[i] == p->nodos[j]) d = 0;
            else for(int k=0; k<len(p->arbol[i]); ++k){
                if(p->arbol[i][k].dest_id == p->nodos[j]){
                    d = p->arbol[i][k].weight;
                    break;
                }
            }
            if(!isfinite(d)) return 0; // Un circuito exige alcanzabilidad mutua.
            p->dist[i][j] = d;
        }
    }
    for(int i=0; i<=p->n; ++i){
        p->entrada_min[i] = p->salida_min[i] = INFINITY;
        for(int j=0; j<=p->n; ++j) if(i != j){
            p->entrada_min[i] = fmin(p->entrada_min[i], p->dist[j][i]);
            p->salida_min[i] = fmin(p->salida_min[i], p->dist[i][j]);
        }
    }
    return 1;
}

static int viaje(double metros){ return (int)ceil(metros / METROS_MINUTO); }

static double multa(const Entrega *e, int minuto){
    return fmax(0, minuto - e->cierre) * TARIFA * (1 + e->priority);
}

// DP por subconjuntos para una cota optimista de las multas pendientes.
// Cada visita usa su entrada mas corta e ignora esperas. Para un subconjunto
// el tiempo relajado es independiente del orden; elegimos su ultima entrega.
static double cota_multas(const Plan *p, int restantes, int minuto){
    double dp[1 << MAXENTREGA] = {0};
    for(int s=1; s<=restantes; ++s){
        if(s & ~restantes) continue;
        int llegada = minuto;
        for(int i=0; i<p->n; ++i)
            if(s & (1<<i)) llegada += viaje(p->entrada_min[i+1]);
        dp[s] = INFINITY;
        for(int i=0; i<p->n; ++i)
            if(s & (1<<i))
                dp[s] = fmin(dp[s], dp[s ^ (1<<i)] + multa(&p->entregas[i], llegada));
    }
    return dp[restantes];
}

static void buscar(Plan *p, int actual, int restantes, int nivel, int minuto, double costo){
    if(!restantes){
        costo += p->dist[actual][0];
        if(costo < p->mejor){
            p->mejor = costo;
            memcpy(p->mejor_orden, p->orden, p->n * sizeof(int));
        }
        return;
    }
    // Toda terminacion sale una vez de actual y de cada entrega pendiente.
    double cota = costo + p->salida_min[actual];
    for(int i=0; i<p->n; ++i)
        if(restantes & (1<<i)) cota += p->salida_min[i+1];
    if(p->flexible) cota += cota_multas(p, restantes, minuto);
    if(cota >= p->mejor) return;
    for(int i=0; i<p->n; ++i){
        if(!(restantes & (1<<i))) continue;
        Entrega *e = &p->entregas[i];
        double d = p->dist[actual][i+1];
        int servicio = (int)fmax(e->apertura, minuto + viaje(d));
        if(!p->flexible && servicio > e->cierre) continue;
        p->orden[nivel] = i+1;
        buscar(p, i+1, restantes ^ (1<<i), nivel+1, servicio, costo+d+multa(e, servicio));
    }
}

static int exportar(const Plan *p, FILE *f){
    fprintf(f, "{\n\"deposito\":%" PRIu64 ",\"salida_min\":%d,\"modo\":\"%s\",\n",
            p->nodos[0], p->inicio, p->flexible ? "penalizar" : "estricto");
    fputs("\"entregas_por_prioridad\":[", f);
    for(int i=0; i<p->n; ++i){
        const Entrega *e = &p->entregas[i];
        fprintf(f, "%s{\"id\":%d,\"nodo\":%" PRIu64 ",\"prioridad\":%d,\"desde_min\":%d,\"hasta_min\":%d}",
                i ? "," : "", e->id, e->nodo, e->priority, e->apertura, e->cierre);
    }
    fputs("],\n\"matriz_m\":[", f);
    for(int i=0; i<=p->n; ++i){
        fputs(i ? ",[" : "[", f);
        for(int j=0; j<=p->n; ++j) fprintf(f, "%s%.6f", j ? "," : "", p->dist[i][j]);
        fputc(']', f);
    }
    fputs("],\n\"tramos\":[", f);
    double metros = 0, penalizacion = 0;
    int origen = 0, minuto = p->inicio;
    for(int k=0; k<=p->n; ++k){
        int dest = k == p->n ? 0 : p->mejor_orden[k];
        double d = p->dist[origen][dest];
        int llegada = minuto + viaje(d);
        minuto = dest ? (int)fmax(llegada, p->entregas[dest-1].apertura) : llegada;
        metros += d;
        if(dest) penalizacion += multa(&p->entregas[dest-1], minuto);
        fprintf(f, "%s{\"origen\":%" PRIu64 ",\"destino\":%" PRIu64 ",\"entrega_id\":",
                k ? "," : "", p->nodos[origen], p->nodos[dest]);
        if(dest) fprintf(f, "%d", p->entregas[dest-1].id);
        else fputs("null", f);
        fprintf(f, ",\"distancia_m\":%.6f,\"llegada_min\":%d,\"servicio_min\":%d,\"aristas\":[", d, llegada, minuto);
        _Dijkstra_edge *ruta;
        int n = ruta_inversa(p->arbol[origen], p->nodos[origen], p->nodos[dest], &ruta);
        if(n < 0) return 0;
        for(int i=n-1; i>=0; --i)
            fprintf(f, "%s[%" PRIu64 ",%" PRIu64 ",%" PRIu64 "]", i == n-1 ? "" : ",",
                    ruta[i].src_id, ruta[i].dest_id, ruta[i].edge_id);
        free(ruta);
        fputs("]}", f);
        origen = dest;
    }
    fprintf(f, "],\n\"distancia_m\":%.6f,\"penalizacion\":%.0f,\"costo\":%.6f}\n",
            metros, penalizacion, p->mejor);
    return !ferror(f);
}

int ejecutar_plan(int argc, char **argv){
    Plan p = {.mejor = INFINITY};
    if((argc != 5 && argc != 6) || !nodo(argv[2], &p.nodos[0]) ||
       (p.inicio = hora(argv[3])) < 0 || (argc == 6 && strcmp(argv[5], "--penalizar"))){
        fputs("Uso: --plan nodo_deposito HH:MM entregas.csv [--penalizar]\n", stderr);
        return 1;
    }
    p.flexible = argc == 6;
    FILE *f = fopen(argv[4], "r");
    if(!f){ perror(argv[4]); return 1; }
    int ok = leer(&p, f);
    fclose(f);
    if(!ok){ fprintf(stderr, "CSV invalido: se esperan 1 a %d entregas con ventanas validas.\n", MAXENTREGA); return 1; }
    mergesort(p.entregas, p.n, entrega_cmp);
    for(int i=0; i<p.n; ++i) p.nodos[i+1] = p.entregas[i].nodo;
    Graph g = initGraph();
    ok = load_graph(&g) && matriz(&p, &g);
    if(!ok) fputs("No hay circuito entre los nodos indicados. Revisa los identificadores y el mapa.\n", stderr);
    else {
        buscar(&p, 0, (1<<p.n)-1, 0, p.inicio, 0);
        ok = isfinite(p.mejor);
        if(!ok) fputs("Ningun circuito cumple todas las ventanas horarias.\n", stderr);
    }
    if(ok){
        f = fopen("ruta.json", "w");
        ok = f && exportar(&p, f);
        if(f && fclose(f)) ok = 0;
        if(!ok) fputs("Error al escribir ruta.json.\n", stderr);
        else printf("Circuito guardado en ruta.json. Costo: %.2f\n", p.mejor);
    }
    for(int i=0; i<=p.n; ++i) if(p.arbol[i]) free(get_meta(p.arbol[i]));
    for(int i=0; i<g.vertex.capacity; ++i){
        Node *n = g.vertex.table[i].neighbors.head;
        while(n){ Node *sig = n->next; free(n); n = sig; }
    }
    free(g.vertex.table);
    return ok ? 0 : 1;
}
