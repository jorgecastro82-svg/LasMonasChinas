import argparse
import json
from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection
import numpy as np


parser = argparse.ArgumentParser(description="Mapa de rutas de Tijuana")
parser.add_argument("--plan", type=Path, help="JSON generado por el planificador")
parser.add_argument("--guardar", type=Path, help="Guardar la figura como imagen")
args = parser.parse_args()
interactivo = bool(args.plan and not args.guardar)
if interactivo:
    from simulacion import Simulacion, TINTA, RUTA, PENDIENTE
root = Path(__file__).resolve().parents[1]

#Load original OSM data
nodes = pd.read_csv(root / "map_data/nodes.csv")
edges = pd.read_csv(root / "map_data/edges.csv", low_memory=False)


#Map each node ID to its coordinates (longitude, latitude)
coords = nodes.set_index("id")[["lon", "lat"]]


#Parse OSM LINESTRING geometry
def parse_linestring(geometry):
	coordinates = geometry.removeprefix("LINESTRING (").removesuffix(")")
	return [
		tuple(map(float, point.split()))
		for point in coordinates.split(", ")
	]


#Build visualization of the entire city
segments = [
	parse_linestring(geometry)
	for geometry in edges["geometry"]
]


if args.plan:
    with args.plan.open(encoding="utf-8") as archivo:
        plan = json.load(archivo)
    path_edges = [edge for tramo in plan["tramos"] for edge in tramo["aristas"]]
else:
    path_edges = [
        (int(row.src), int(row.dest), int(row.way_id))
        for row in pd.read_csv(root / "dijkstra_output.csv").itertuples(index=False)
    ]


#Retrieve the exact OSM geometry for each path edge
path_segments = []

# Index the original geometries once; a long circuit can reuse the same street.
indices = edges.groupby(["u", "v", "id"], sort=False).indices
filas_ruta = []
for clave in map(tuple, path_edges):
    filas = indices.get(clave)
    if filas is None:
        raise ValueError(f"Edge not found: {clave}")
    fila = filas[0]
    filas_ruta.append(fila)
    path_segments.append(segments[fila])


# Obtain path vertices for start and destination markers
path = ([path_edges[0][0]] + [edge[1] for edge in path_edges]) if path_edges else []
if args.plan and not path:
    path = [plan["deposito"]]

path_lon = [coords.loc[node, "lon"] for node in path]
path_lat = [coords.loc[node, "lat"] for node in path]


# Calculate longitude correction for aspect ratio
aspect_ratio = 1 / np.cos(np.deg2rad(nodes["lat"].mean()))


# Create figure
fig, ax = plt.subplots(figsize=(14, 12))


# Entire city road network
ax.add_collection(LineCollection(
	segments,
	colors="lightgray",
	linewidths=0.4
))


# Shortest path visualization
ax.add_collection(LineCollection(
	path_segments,
	colors=RUTA if interactivo else "red",
	linewidths=2.5,
	label="Ruta optimizada" if args.plan else "Camino"
))


# Start and destination markers
if path:
    ax.scatter(path_lon[0], path_lat[0], color=TINTA if interactivo else "green", s=90,
               marker="D" if interactivo else "o",
               label="Depósito" if args.plan else "Inicio", zorder=3)
if interactivo:
    ax.annotate("Depósito", (path_lon[0], path_lat[0]), xytext=(-9, 9),
                textcoords="offset points", ha="right", fontsize=9, color=TINTA,
                bbox=dict(facecolor="white", edgecolor="none", alpha=.9, pad=1), zorder=5)
if args.plan:
    etiquetas = {}
    for numero, tramo in enumerate(plan["tramos"][:-1], start=1):
        texto = f"{numero}. Entrega {tramo['entrega_id']}"
        etiquetas.setdefault(tramo["destino"], []).append(texto)
    # Separar las etiquetas de sur a norte para leer paradas muy próximas.
    ubicaciones = sorted(etiquetas, key=lambda nodo: coords.at[nodo, "lat"])
    for fila, nodo in enumerate(ubicaciones):
        lon, lat = coords.loc[nodo]
        ax.scatter(lon, lat, facecolors=PENDIENTE if interactivo else "none",
                   edgecolors="white" if interactivo else "blue", s=83 if interactivo else 100, zorder=4)
        offset = 18 * (fila - (len(ubicaciones) - 1) / 2)
        ax.annotate("\n".join(etiquetas[nodo]), (lon, lat), xytext=(24, offset),
                    textcoords="offset points", va="center", color="navy", zorder=5,
                    bbox=dict(facecolor="white", edgecolor="none", alpha=0.9, pad=1.5),
                    arrowprops=dict(arrowstyle="-", color="navy", lw=0.7))
elif path:
    ax.scatter(path_lon[-1], path_lat[-1], color="blue", s=80, label="Destino", zorder=3)


# Display road information
if not interactivo:
    print("\nRoad information:")

    for (src, dest, way_id), fila in zip(path_edges, filas_ruta):
        edge = edges.iloc[fila]
        print(f"\n{src} -> {dest}")
        print(f"Way ID: {way_id}")
        print(f"Name: {edge['name']}")
        print(f"Highway: {edge['highway']}")
        print(f"Max speed: {edge['maxspeed']}")
        print(f"Surface: {edge['surface']}")
        print(f"Length: {edge['length']} m")


# Configure plot
ax.autoscale()
ax.margins(0.02)
ax.set_aspect(aspect_ratio)

if not interactivo:
    ax.set_title("Trayectoria Repartidor")
ax.set_xlabel("Longitud")
ax.set_ylabel("Latitud")
ax.legend()

if args.guardar:
    fig.savefig(args.guardar, dpi=140, bbox_inches="tight")
else:
    if interactivo:
        simulacion = Simulacion(fig, ax, plan, path_segments, coords)
    plt.show()
