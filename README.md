# LasMonasChinas

Sistema de planificación y simulación de rutas de reparto en las calles de Tijuana, utilizando datos de OpenStreetMap, algoritmos implementados en C y visualización en Python.

## Características

- Planificación de hasta cuatro entregas.
- Prioridades alta, media y baja.
- Ventanas horarias de entrega.
- Optimización de rutas y distancias.
- Simulación visual del recorrido.
- Regreso al punto de partida.

## Algoritmos utilizados

- **Dijkstra:** cálculo de caminos más cortos.
- **Merge Sort:** ordenamiento de entregas por prioridad.
- **Branch and Bound:** búsqueda de la ruta óptima.
- **Programación dinámica:** cálculo de cotas para penalizaciones por retraso.

## Requisitos

- GCC compatible con C11.
- Python.
- Pandas, NumPy y Matplotlib.

Instalar las dependencias:

```bash
python -m pip install pandas numpy matplotlib
```

## Compilación

Desde PowerShell, en la carpeta principal del proyecto:

```powershell
gcc -std=c11 -O2 main.c planificador.c merge.c entrega_repartidor/entrega.c graph/graph.c graph/dijkstra.c graph/hashtable.c graph/linkedlist.c stringy_buffer/stringy.c -o planificador.exe -lm
```

## Ejecución

Calcular la ruta:

```powershell
.\planificador.exe --plan 5075349680 08:00 entregas_ejemplo.csv
```

Iniciar la visualización:

```powershell
python visualization/main.py --plan ruta.json
```

## Configuración de entregas

Las entregas se configuran en `entregas
