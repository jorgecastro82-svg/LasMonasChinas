
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include "graph/dijkstra.h"

#define EARTH_RADIUS 6371000.0
#define PI 3.14159265358979323846

//Geocoding functions and read csvs then put all edges and vertexes into a graph
double haversine(double lat1, double lon1, double lat2, double lon2);
int find_closest_node(double target_lat, double target_lon, uint64_t *closest_id);
int load_graph(Graph *g);
int write_route(_Dijkstra_edge *fixed, uint64_t source, uint64_t destination);


int main(int argc, char *argv[]){
    //#if(0)
	double src_lat = strtod(argv[1], NULL);
	double src_lon = strtod(argv[2], NULL);
	double dest_lat = strtod(argv[3], NULL);
	double dest_lon = strtod(argv[4], NULL);
    //#endif
	uint64_t source, destination;
    //THIS IS FOR TESTS
    /*
    uint64_t source = 181289986;
    uint64_t destination = 4838183345;
    */
    //#if(0)
	//Find closest OSM nodes
	if(!find_closest_node(src_lat, src_lon, &source)){
		printf("Could not find source node.\n");
		return 1;
	}

	if(!find_closest_node(dest_lat, dest_lon, &destination)){
		printf("Could not find destination node.\n");
		return 1;
	}
    //#endif
	printf("\nSource: %" PRIu64 "\n", source);
	printf("Destination: %" PRIu64 "\n\n", destination);

	// Build graph
	Graph graph = initGraph();

	if(!load_graph(&graph)){
		printf("Could not load graph.\n");
		return 1;
	}

	printf("Graph loaded.\n");

	// Run Dijkstra
	_Dijkstra_edge *fixed = dijkstra(&graph, source);

	// Reconstruct and export route
	int success = write_route(fixed, source, destination);

	// Free Dijkstra's stretchy buffer
	if(fixed)
		free(get_meta(fixed));

	return success ? 0 : 1;
}

double haversine(double lat1, double lon1, double lat2, double lon2){
	double to_rad = PI / 180.0;

	lat1 *= to_rad;
	lon1 *= to_rad;
	lat2 *= to_rad;
	lon2 *= to_rad;

	double dlat = lat2 - lat1;
	double dlon = lon2 - lon1;

	double a = pow(sin(dlat / 2), 2) +
		cos(lat1) * cos(lat2) * pow(sin(dlon / 2), 2);

	double c = 2 * atan2(sqrt(a), sqrt(1 - a));

	return EARTH_RADIUS * c;
}


int find_closest_node(double target_lat, double target_lon, uint64_t *closest_id){
	FILE *file = fopen("map_data/nodes_simple.csv", "r");
	if(!file) return 0;

	char line[256];

	// Skip CSV header
	fgets(line, sizeof(line), file);

	double min_distance = INFINITY;
	uint64_t best_id = 0;

	while(fgets(line, sizeof(line), file)){
		uint64_t id;
		double lon, lat;

		if(sscanf(line, "%" SCNu64 ",%lf,%lf", &id, &lon, &lat) != 3)
			continue;

		double distance = haversine(target_lat, target_lon, lat, lon);

		if(distance < min_distance){
			min_distance = distance;
			best_id = id;
		}
	}

	fclose(file);

	if(min_distance == INFINITY) return 0;

	*closest_id = best_id;

	printf("Closest node: %" PRIu64 " (%.2f meters away)\n",
		best_id, min_distance);

	return 1;
}


int load_graph(Graph *g){
	FILE *file = fopen("map_data/edges_simple.csv", "r");
	if(!file){
		perror("Could not open edges_simple.csv");
		return 0;
	}

	char line[256];

	// Skip CSV header
	fgets(line, sizeof(line), file);

	uint64_t src, dest, way_id;
	float weight;

	while(fgets(line, sizeof(line), file)){
		if(sscanf(line, "%" SCNu64 ",%" SCNu64 ",%f,%" SCNu64,
			&src, &dest, &weight, &way_id) != 4)
			continue;

		add_vertex(g, src);
		add_vertex(g, dest);

		add_edge(g, src, (Edge){
			.weight = weight,
			.dest_id = dest,
			.edge_id = way_id
		});
	}

	fclose(file);
	return 1;
}


int write_route(_Dijkstra_edge *fixed, uint64_t source, uint64_t destination){
	FILE *file = fopen("dijkstra_output.csv", "w");
	if(!file){
		perror("Could not create dijkstra_output.csv");
		return 0;
	}

	fprintf(file, "src,dest,way_id\n");

	// Source and destination are the same node
	if(source == destination){
		fclose(file);
		return 1;
	}

	if(!fixed){
		printf("No route found.\n");
		fclose(file);
		return 0;
	}

	int count = len(fixed);

	_Dijkstra_edge *route = malloc(count * sizeof(_Dijkstra_edge));
	if(!route){
		fclose(file);
		return 0;
	}

	int route_size = 0;
	uint64_t current = destination;

	// Reconstruct the route backwards
	while(current != source && route_size < count){
		int found = 0;

		for(int i = 0; i < count; i++){
			if(fixed[i].dest_id == current){
				route[route_size++] = fixed[i];
				current = fixed[i].src_id;
				found = 1;
				break;
			}
		}

		if(!found){
			printf("Could not reconstruct route.\n");
			free(route);
			fclose(file);
			return 0;
		}
	}

	// Write route forwards
	for(int i = route_size - 1; i >= 0; i--){
		fprintf(file, "%" PRIu64 ",%" PRIu64 ",%" PRIu64 "\n",
			route[i].src_id,
			route[i].dest_id,
			route[i].edge_id);
	}

	free(route);
	fclose(file);

	printf("Route written to dijkstra_output.csv\n");
	printf("Number of edges: %d\n", route_size);

	return 1;
}
