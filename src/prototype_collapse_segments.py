#!/usr/bin/env python

# testing out the idea of collapsing extracted segments from OSM
#
# input, csv with columns:
# - way_id, segment_number, start_node_id, end_node_id
#
# output, to STDOUT, comma-separated list:
# - osm_way_id, osm_way_id_segment_number, start_node_id, end_node_id

import sys, csv, json

input_filename = sys.argv[1]

nodes = {} # node_id : list of edge_ids
edges = {} # edge_id : osm_way_id, osm_way_id_segment_number, start_node_id, end_node_id

with open(input_filename, 'r') as input_file:
	r = csv.reader(input_file)
	for line in r:
		# way_id, segment_number, start_node_id, end_node_id
		edge_id = line[0]+"_"+line[1]
		start_id = line[2]
		end_id = line[3]

		# store edges by derivative way id / segment number
		edges[edge_id] = line

		# store list of edges intersecting node
		if(start_id in nodes):
			nodes[start_id].append(edge_id)
		else:
			nodes[start_id] = [edge_id]

		if(end_id in nodes):
			nodes[end_id].append(edge_id)
		else:
			nodes[end_id] = [edge_id]

def merge_edges(a,b,node):
	nodes = set([a[2],b[2],a[3],b[3]])
	nodes.remove(node)
	return [a[0],a[1]] + list(nodes)

def merge_node_edges(edge_ids,to_remove,to_add):
	edge_ids = set(edge_ids)
	edge_ids = edge_ids - set(to_remove)
	edge_ids.add(to_add)
	return list(edge_ids)

for node in nodes:
	node_edges = nodes[node]
	if (len(node_edges) == 2):
		a = edges[node_edges[0]]
		b = edges[node_edges[1]]
		merged = merge_edges(a,b,node)
		# merged id is id of a
		merged_id = node_edges[0]
		merged_start = merged[2]
		merged_end = merged[3]
		# update a to merged (a+b)
		edges[merged_id] = merged
		# delete b
		del edges[node_edges[1]]
		# update this node to have no edges
		nodes[node] = []
		# update merged start node list to point to merged
		nodes[merged_start] = merge_node_edges(nodes[merged_start], node_edges, merged_id)
		nodes[merged_end] = merge_node_edges(nodes[merged_end], node_edges, merged_id)


for edge_id, edge in edges.items():
	print ",".join(edge)
