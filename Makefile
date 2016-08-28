# to summarise the contents of an .osm.pbf file
build/osm_pbf_summary:
	g++ src/osm_pbf_summary.cpp -std=c++11 -O2 -I include -pthread -lz \
	-o build/osm_pbf_summary

# to split an .osm.pbf file into highway segments
build/osm_to_segments:
	g++ src/osm_to_segments.cpp -std=c++11 -O2 -I include -pthread -lz \
	-o build/osm_to_segments

build/osm_to_segments_small:
	g++ src/osm_to_segments_small.cpp -std=c++11 -O2 -I include -pthread -lz \
	-o build/osm_to_segments_small

# to extract the coordinates of a list of nodes from an .osm.pbf file
build/osm_pbf_node_coordinates:
	g++ src/osm_pbf_node_coordinates.cpp -std=c++11 -O2 -I include -pthread -lz \
	-o build/osm_pbf_node_coordinates

# to run incremental connected components on a text file list of edges
build/percolation_incremental_connected_components:
	g++ src/percolation_incremental_connected_components.cpp -std=c++11 -O2 \
	-lboost_system -o build/percolation_incremental_connected_components

# to run a single threshold of connected components on a text file list of edges
build/percolation_connected_components:
	g++ src/percolation_connected_components.cpp -std=c++11 -O2 -lboost_system -o \
	build/percolation_connected_components

# extract segments from Europe .osm.pbf
data/europe_segments.txt: pbf_segments
	./build/pbf_segments data/europe-20160621.osm.pbf data/europe_segments.txt \
	data/europe_nodes.txt

##
# Load segments into Postgres database (see ./sql for scripts)
# Collapse segments according to endpoint degree (see python script in ./sql,
# or note prototype in ./src/prototype_collapse_segments.py)
# #

# extract text file list of all edges from Postgres for percolation/visualisation
data/europe_edges.tsv:
	psql -c "COPY osm_segments_min (start_node_id, end_node_id, length) TO
	'~/casa/dissertation/dev/src/data/europe_edges.tsv.unsorted';"
	sort -n -k 3 data/europe_edges.tsv.unsorted > data/europe_edges.tsv; \
	rm -f data/europe_edges.tsv.unsorted

# extract text file list of edges from Postgres, limited to Greater London
# bounding box, for percolation/visualisation
data/london_edges.tsv:
	psql -c "COPY (select start_node_id, end_node_id, length from
	osm_segments_min WHERE geog && ST_MakeEnvelope(-1.115, 50.941, 0.895,
	51.984) ) TO '~/casa/dissertation/dev/src/data/london_edges.tsv.unsorted';"
	sort -n -k 3 data/london_edges.tsv.unsorted > data/london_edges.tsv; \
	rm -f data/london_edges.tsv.unsorted

# extract all unique 'start' nodes from Postgres
data/nodes_s.tsv:
	psql -c "COPY (select start_node_id, count(*), min(length) from
	osm_segments_min group by start_node_id order by start_node_id) TO
	'~/casa/dissertation/dev/src/data/nodes_s.tsv';"

# extract all unique 'end' nodes from Postgres
data/nodes_e.tsv:
	psql -c "COPY (select end_node_id, count(*), min(length) from
	osm_segments_min group by end_node_id order by end_node_id) TO
	'~/casa/dissertation/dev/src/data/nodes_e.tsv';"

# sort and merge start and end nodes, maintaining
# 1) count of the degree of each node
# 2) minimum length of any edge coinciding with each node
# columns:
# id count length
data/europe_nodes.tsv: data/nodes_s.tsv data/nodes_e.tsv
	sort -mn data/nodes_s.tsv data/nodes_e.tsv > data/nodes_se.tsv; \
	python merge_nodes.py data/nodes_se.tsv data/europe_nodes.tsv

# extract segment endpoint node ids from list of nodes
data/europe_node_ids.tsv:
	cut -f 1 data/europe_nodes.tsv > data/europe_node_ids.tsv

# extract coordinates of nodes
data/europe_node_coords.tsv: pbf_node_ids
	./osm_extract_node_coords data/europe_node_ids.tsv \
	data/europe-20160621.osm.pbf > data/europe_node_coords.tsv

# join node coordinates with degree (count) and minimum adjacent edge length
# columns:
# id count length POINT(lon lat)
data/europe_nodes_with_coords.tsv:
	join -j 1 --nocheck-order data/europe_nodes.tsv data/europe_node_coords.tsv \
	 > data/europe_nodes_with_coords.tsv

##
# Load nodes into osm_nodes table in Postgres (see sql scripts)
# #

# extract text file list of nodes from Postgres, limited to Greater London
# bounding box
data/london_nodes.tsv:
	psql -c "COPY (select id, ST_AsLatLonText(Geometry(geog), 'D.DDDDDDD') from
	osm_nodes WHERE geog && ST_MakeEnvelope(-1.115, 50.941, 0.895, 51.984) ) TO
	'~/casa/dissertation/dev/src/data/london_nodes.tsv.unsorted';"
	sort -k 1b,1 data/london_nodes.tsv.unsorted | sed 's/ /\t/' | \
	awk '{ print $$1 "\t" $$3 "\t" $$2 }' > data/london_nodes.tsv; \
	rm -f data/london_nodes.tsv.unsorted

# reformat nodes to geojson point feature format, for tippecanoe
data/europe_nodes.geojson: data/europe_nodes_with_coords.tsv
	python src/nodes_to_geojson.py

# process geojson nodes into vector tiles for web visualisation
data/europe_nodes.mbtiles: data/europe_nodes.geojson
	tippecanoe -o data/europe_nodes.mbtiles data/europe_nodes.geojson \
	-l europe_nodes --maximum-zoom=14 --minimum-zoom=2 --no-feature-limit \
	--no-tile-size-limit --base-zoom=12 --read-parallel -rg

# remove built/compiled scripts
.PHONY: clean
clean:
	rm build/*
