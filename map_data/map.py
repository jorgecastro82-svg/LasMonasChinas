import pandas as pd

from pyrosm import OSM

# Obtaining the CSV of edges and nodes of Tijuana
# bbox = [-117.05, 32.45, -116.85, 32.55]

# osm = OSM("mexico.pbf", bounding_box=bbox, keep_metadata=False, engine="out_of_core")
# nodes, edges = osm.get_network(network_type="driving", nodes=True)

# nodes.to_csv("nodes.csv", index=False)
# edges.to_csv("edges.csv", index=False)


# Simplification of edges.csv
edges = pd.read_csv("edges.csv", low_memory=False)

edges_simple = edges[["u", "v", "length", "id"]].copy()
edges_simple.columns = ["src", "dest", "weight", "way_id"]

edges_simple.to_csv("edges_simple.csv", index=False)


# Simplification of nodes.csv
nodes = pd.read_csv("nodes.csv")

nodes_simple = nodes[["id", "lon", "lat"]].copy()

nodes_simple.to_csv("nodes_simple.csv", index=False)
