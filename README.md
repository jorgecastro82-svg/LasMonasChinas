# LasMonasChinas

Planificador de rutas de reparto sobre el grafo dirigido de calles de Tijuana de OpenStreetMap. C carga las listas de adyacencia, calcula la ruta y exporta los resultados. Python dibuja las geometrías originales de las calles.

## Ejecutar en Visual Studio Code (Windows)

Abre la carpeta del proyecto y una terminal PowerShell en su raíz. Para compilar con TDM-GCC de 64 bits:

~~~powershell
& 'C:/TDM-GCC-64/bin/gcc.exe' -std=c11 -O2 -Wall -Wextra main.c planificador.c merge.c entrega_repartidor/entrega.c graph/graph.c graph/dijkstra.c graph/hashtable.c graph/linkedlist.c stringy_buffer/stringy.c -o planificador.exe -lm
~~~

Con otro GCC de 64 bits compatible con C11, usa la misma lista de fuentes. Instala las dependencias del visor una vez con `python -m pip install pandas numpy matplotlib`.

~~~powershell
.\planificador.exe --plan 5075349680 08:00 entregas_ejemplo.csv
python visualization/main.py --plan ruta.json
~~~

Para guardar una imagen, añade `--guardar mapa.png` al comando de Python. Abre el JSON del último cálculo exitoso. El modo original sigue disponible y escribe `dijkstra_output.csv`:

~~~powershell
.\planificador.exe 32.5389793 -116.9374106 32.5390961 -116.9373339
python visualization/main.py
~~~

## Registrar entregas

Edita una copia de `entregas_ejemplo.csv`. Conserva la cabecera `nodo,prioridad,desde,hasta`. Usa el ID de una intersección de `map_data/nodes_simple.csv`, prioridad 0 (baja), 1 (media) o 2 (alta), y dos horas HH:MM del mismo día. Cada fila recibe un ID consecutivo desde 0. El depósito se indica en la línea de comandos. Se admiten de una a cuatro entregas, según la capacidad MAXENTREGA del equipo. Varias entregas pueden compartir nodo.

## Cálculo e interpretación

MergeSort ordena por prioridad descendente y conserva el orden de registro en empates. Dijkstra obtiene una fila de la matriz dirigida por parada. La fila y columna 0 corresponden al depósito; las demás siguen el arreglo JSON `entregas_por_prioridad`. Los árboles de predecesores también permiten reconstruir las calles de cada tramo.

Branch and Bound busca el circuito de distancia mínima que visita todas las entregas y regresa al depósito dentro del grafo cargado. Para podar, suma al costo parcial la salida más corta del nodo actual y de cada parada pendiente. El orden de prioridad se usa para explorar alternativas y desempatar circuitos del mismo costo. La secuencia final aparece en `tramos`.

El ejemplo usa 30 km/h constantes (500 m/min), redondeo hacia arriba por tramo y servicio instantáneo. Si el repartidor llega temprano, espera hasta la apertura. En modo estricto el servicio debe empezar a más tardar al cierre. Si las ventanas son incompatibles o faltan conexiones para regresar al depósito, el programa informa el problema y termina con error.

Para permitir retrasos, añade `--penalizar` al comando de C. El costo es distancia en metros más 100 × (prioridad + 1) por minuto de atraso. Una DP por subconjuntos calcula una cota inferior de las multas futuras suponiendo la entrada más corta a cada parada y omitiendo esperas. Branch and Bound añade esta cota antes de podar. La tarifa y la velocidad son parámetros del modelo académico.

El JSON contiene la matriz en metros, las entregas ordenadas, los horarios en minutos desde medianoche, las aristas OSM de cada tramo, la distancia total, la penalización y el costo. El visor numera las paradas según el orden de visita.
