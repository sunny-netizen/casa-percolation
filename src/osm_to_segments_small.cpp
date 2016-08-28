#include <list>
#include <iostream>
#include <fstream>

// to parse pbf files
#include <osmium/io/pbf_input.hpp>

#include "print_utils.hpp"
#include "waynode.hpp"
#include "waysegment.hpp"


// to store node locations, by id:

// a) dense file array may be more compact for whole planet/continent
// #include <osmium/index/map/dense_file_array.hpp>
// typedef osmium::index::map::DenseFileArray<osmium::unsigned_object_id_type, WayNode> index_location_type;

// b) sparse mem table is faster/smaller for small extracts
#define OSMIUM_WITH_SPARSEHASH // requires google sparsehash library for sparse_mem_table
#include <osmium/index/map/sparse_mem_table.hpp>
typedef osmium::index::map::SparseMemTable<osmium::unsigned_object_id_type, WayNode> index_location_type;


void save_node_to_index(const osmium::Node &node, index_location_type &way_nodes){
	auto id = node.positive_id();
	way_nodes.set(id, WayNode(id, node.location()));
}

bool check_way(const osmium::Way &way){
	const char* highway = way.tags().get_value_by_key("highway");
	if(!highway){
		return false;
	}
	// see http://wiki.openstreetmap.org/wiki/Highways for tag semantics
	if (
			strcmp(highway, "motorway")
			|| strcmp(highway, "motorway_link")
			|| strcmp(highway, "motorway_junction")
			|| strcmp(highway, "trunk")
			|| strcmp(highway, "trunk_link")
			|| strcmp(highway, "primary")
			|| strcmp(highway, "primary_link")
			|| strcmp(highway, "secondary")
			|| strcmp(highway, "secondary_link")
			|| strcmp(highway, "tertiary")
			|| strcmp(highway, "tertiary_link")
			|| strcmp(highway, "residential")
			|| strcmp(highway, "road")
			|| strcmp(highway, "unclassified")
			|| strcmp(highway, "living_street")
			|| strcmp(highway, "service")
			|| strcmp(highway, "pedestrian")
		) {
			return true;
	} else {
		return false;
	}
}

void add_node_count(const osmium::Way &way, index_location_type &way_nodes){
	if(!check_way(way)){
		return;
	}
	for (auto& node_ref : way.nodes()) {
		auto id = node_ref.positive_ref();

		try {
			auto way_node = way_nodes.get(id);
			// way_node.add_way(way.id());
			way_node.inc();
			way_nodes.set(id, way_node);
		} catch (osmium::not_found&) {
			std::cerr << "No node for "<< id << std::endl;
		}

	}
}

std::list<WaySegment> way_to_segments(osmium::Way &way, const index_location_type &way_nodes){
	std::list<WaySegment> segments;
	if(!check_way(way)){
		return segments;
	}
	int i = 0;
	int seg = 0;
	WaySegment segment;
	int last = way.nodes().size() - 1;
	osmium::unsigned_object_id_type way_id = way.positive_id();

	for (auto& node_ref : way.nodes()) {
		auto id = node_ref.positive_ref();
		int count = 0;

		try {
			auto way_node = way_nodes.get(id);
			count = way_node.count();
			node_ref.set_location(way_node.location());
		} catch (osmium::not_found&) {
			std::cerr << "No location for "<< id << std::endl;
			node_ref.set_location(osmium::Location());
		}

		if(i == 0){
			// first - start segment
			segment = WaySegment(way_id, seg);
			segment.addNode(node_ref);
		} else {
			if(i == last){
				// last - end segment
				segment.addNode(node_ref);
				segments.push_back(segment);
			} else {
				if(count > 1){
					// end/start segment
					segment.addNode(node_ref);
					segments.push_back(segment);

					++seg;

					segment = WaySegment(way_id, seg);
					segment.addNode(node_ref);
				} else {
					// continue segment
					segment.addNode(node_ref);
				}
			}
		}
		++i;
	}
	return segments;
}

int main (int argc, char* argv[]) {
	try {
		// check arguments, expect a filename
		if (argc != 4) {
			std::cerr << "Usage: " << argv[0] << " filename.osm.pbf out_segments.txt out_nodes.txt" << std::endl;
			exit(1);
		}

		// Read from file passed as argument, only nodes and ways
		osmium::io::Reader node_reader(argv[1], osmium::osm_entity_bits::node);
		osmium::io::Reader way_reader(argv[1], osmium::osm_entity_bits::way);
		osmium::io::Reader way_segment_reader(argv[1], osmium::osm_entity_bits::way);

		std::ofstream segment_file;
		segment_file.open(argv[2]);

		// set up map to store node locations
		index_location_type way_nodes;
		//way_nodes.reserve(4000000000);

		// Read nodes
		int node_total = 1;
		while (osmium::memory::Buffer buffer = node_reader.read()) {
			for (auto entity = std::begin(buffer), end = std::end(buffer); entity != end; ++entity) {
				osmium::Node & node = static_cast<osmium::Node &>(*entity);
				save_node_to_index(node, way_nodes);
				node_total++;
				if(node_total % 1000000 == 0){
					std::cerr << "Read " << node_total << " nodes." << std::endl;
				}
			}
		}
		node_reader.close();
		std::cerr << "Done reading " << node_total << " nodes." << std::endl;

		// Read ways
		int way_total = 0;
		int way_node_total = 0;
		while (osmium::memory::Buffer buffer = way_reader.read()) {
			for (auto entity = std::begin(buffer), end = std::end(buffer); entity != end; ++entity) {
				add_node_count(static_cast<osmium::Way &>(*entity), way_nodes);
				way_total++;
				if(way_total % 1000000 == 0){
					std::cerr << "Read " << way_total << " ways." << std::endl;
				}
			}
		}
		way_reader.close();
		std::cerr << "Done reading " << way_total << " ways." << std::endl;

		// read ways + write segments
		way_total = 0;
		while (osmium::memory::Buffer buffer = way_segment_reader.read()) {
			for (auto entity = std::begin(buffer), end = std::end(buffer); entity != end; ++entity) {
				std::list<WaySegment> segments = way_to_segments(static_cast<osmium::Way &>(*entity), way_nodes);

				for (std::list<WaySegment>::iterator it = segments.begin(); it != segments.end(); it++){
					WaySegment segment = static_cast<WaySegment>(*it);

					// to print start, end, way_id:
					std::list<osmium::NodeRef> nodes = segment.nodes();
					segment_file << nodes.front().ref() << "\t" << nodes.back().ref();
					segment_file << "\t{" << segment.wayId() << "}\t";

					// to print all node ids
					segment_file << "{";
					bool first = true;

					for (std::list<osmium::NodeRef>::iterator node_it = nodes.begin(); node_it != nodes.end(); node_it++){
						if(first){
							first = false;
						} else {
							segment_file << ",";
						}
						osmium::NodeRef node = static_cast<osmium::NodeRef>(*node_it);
						segment_file << node.ref();
					}
					segment_file << "}";

					// to print linestring
					segment_file << "\tLINESTRING(";
					first = true;

					for (std::list<osmium::NodeRef>::iterator node_it = nodes.begin(); node_it != nodes.end(); node_it++){
						if(first){
							first = false;
						} else {
							segment_file << ", ";
						}
						osmium::NodeRef node = static_cast<osmium::NodeRef>(*node_it);
						auto id = node.ref();
						auto way_node = way_nodes.get(id);

						char buffer [30];
						int cx = snprintf(buffer, 30, "%.7f %.7f", node.location().lon(), node.location().lat());
						if (cx>=0 && cx<=30){
							segment_file << buffer;
						} else {
							std::cerr << "Location string buffer failed " << node.ref();
						}
					}
					segment_file << ")";

					// newline
					segment_file << std::endl;
				}
				way_total++;
				if(way_total % 1000000 == 0){
					std::cerr << "Converted " << way_total << " ways to segments." << std::endl;
				}
			}
		}
		std::cerr << "Done converting " << way_total << " ways to segments." << std::endl;
		way_segment_reader.close();

		// Print summary
		PBFPrinter::print_mem_usage();

		return 0;

	} catch(const std::runtime_error& e) {

		fprintf(stderr, "Failed due to runtime error: %s\n", e.what());
		exit(EXIT_FAILURE);

	}
}
