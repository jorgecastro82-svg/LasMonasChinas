import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection
import numpy as np


#Load original OSM data
nodes = pd.read_csv("../map_data/nodes.csv")
edges = pd.read_csv("../map_data/edges.csv")


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


#Replace with actual Dijkstra output
#Each tuple: (src, dest, way_id)
path_edges = [
	(int(row.src), int(row.dest), int(row.way_id))
	for row in pd.read_csv("../dijkstra_output.csv").itertuples(index=False)
]


#Retrieve the exact OSM geometry for each path edge
path_segments = []

for src, dest, way_id in path_edges:
	edge = edges[
		(edges["u"] == src) &
		(edges["v"] == dest) &
		(edges["id"] == way_id)
	]

	if edge.empty:
		raise ValueError(f"Edge not found: {src} -> {dest}, way {way_id}")

	path_segments.append(parse_linestring(edge.iloc[0]["geometry"]))


# Obtain path vertices for start and destination markers
path = [path_edges[0][0]] + [edge[1] for edge in path_edges]

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
	colors="red",
	linewidths=2.5,
	label="Camino"
))


# Start and destination markers
ax.scatter(
	path_lon[0],
	path_lat[0],
	color="green",
	s=80,
	label="Inicio",
	zorder=3
)

ax.scatter(
	path_lon[-1],
	path_lat[-1],
	color="blue",
	s=80,
	label="Destino",
	zorder=3
)


# Display road information
print("\nRoad information:")

for src, dest, way_id in path_edges:
	edge = edges[
		(edges["u"] == src) &
		(edges["v"] == dest) &
		(edges["id"] == way_id)
	].iloc[0]

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

ax.set_title("Trayectoria Repartidor")
ax.set_xlabel("Longitud")
ax.set_ylabel("Latitud")
ax.legend()

plt.show()
