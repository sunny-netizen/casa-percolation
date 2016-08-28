#ifndef PBF_PRINTER_H
#define PBF_PRINTER_H

// to parse pbf input - pulls in osmium types
#include <osmium/io/pbf_input.hpp>

// to calculate haversine distance
#include <osmium/geom/haversine.hpp>

// for tracking memory usage
#include <osmium/util/memory.hpp>

namespace PBFPrinter {

	void print_header(osmium::io::Header header){
		std::cout << "Header:" << std::endl;

		std::cout << "  Bounding boxes:" << std::endl;
		for (const auto& box : header.boxes()) {
			std::cout << "    " << box << std::endl;
		}
		std::cout << "  With history: " << (header.has_multiple_object_versions() ? "yes" : "no") << std::endl;

		std::cout << "  Options:" << std::endl;
		for (const auto& option : header) {
			std::cout << "    " << option.first << "=" << option.second << std::endl;
		}
	}

	void print_counts(int number_of_nodes, int number_of_ways, int number_of_relations, int number_of_others){
		std::cout << "Counts:" << std::endl;

		std::cout << "  nodes:" << number_of_nodes << std::endl;
		std::cout << "  ways:" << number_of_ways << std::endl;
		std::cout << "  relations:" << number_of_relations << std::endl;
		std::cout << "  others:" << number_of_others << std::endl;
	}

	void print_node(const osmium::Node &node){
		// check basic properties
		std::cout << "Node id: " << node.id() << std::endl;
		std::cout << "  deleted: " << node.deleted() << std::endl;
		std::cout << "  visible: " << node.visible() << std::endl;

		// loop through tags
		std::cout << "  Tags:" << std::endl;
		for (const auto& tag : node.tags()) {
			std::cout << "    " << tag.key() << "=" << tag.value() << std::endl;
		}

		std::cout << "  Location: " << node.location() << std::endl;
	}

	void print_way(const osmium::Way &way){
		// check basic properties
		std::cout << "Way id: " << way.id() << std::endl;
		std::cout << "  deleted: " << way.deleted() << std::endl;
		std::cout << "  visible: " << way.visible() << std::endl;

		// loop through tags
		std::cout << "  Tags:" << std::endl;
		for (const osmium::Tag& tag : way.tags()) {
			std::cout << "    " << tag.key() << "=" << tag.value() << std::endl;
		}

		// get tag value by key
		const char* highway = way.tags().get_value_by_key("highway");
		if (highway && !strcmp(highway, "primary")) {
			// strcmp in c++ returns 0 if two strings are equal
			std::cout << "    highway, primary" << std::endl;
		}

		// loop through nodes
		std::cout << "  Nodes:" << std::endl;
		for (const osmium::NodeRef& nr : way.nodes()) {
			std::cout << "    ref=" << nr.ref() << " location=" << nr.location() << std::endl;
		}

		const double length = osmium::geom::haversine::distance(way.nodes());
		std::cout << "  Length: " << length << std::endl;
	}

	void print_mem_usage(){
		// record peak memory
		osmium::MemoryUsage memory_usage;
		if (memory_usage.peak()) {
			std::cerr << "Memory peak: " << memory_usage.peak() << " MBytes"  << std::endl;
		}
	}

	void print_coords(const osmium::Location &location){
		printf("%.7f,%.7f", location.lon(), location.lat());
	}

	void print_coords_for_wkt(const osmium::Location &location){
		printf("%.7f %.7f", location.lon(), location.lat());
	}

	void print_way_geojson(const osmium::Way &way){
		bool error = false;
		int i = 0;
		int seg = 1;
		int last = way.nodes().size() - 1;
		osmium::unsigned_object_id_type way_id = way.id();

		for (auto& node_ref : way.nodes()) {
			auto id = static_cast<osmium::unsigned_object_id_type>(node_ref.ref());
			int count = 0;
			osmium::Location location(0,0);
			// TODO get location from noderef

			if(i == 0){
				// first - start segment
				std::cout << "{\"type\":\"Feature\",\"properties\":{\"osm_id\":\"" << way_id << "_" << seg << "\"},";
				std::cout << "\"geometry\":{\"type\":\"LineString\",\"coordinates\":[";
				std::cout << "[";
				print_coords(location);
				std::cout << "],";
			} else {
				if(i == last){
					// last - end segment
					std::cout << "[";
					print_coords(location);
					std::cout << "],"; //no trailing comma
					std::cout << "]}}," << std::endl;
				} else {
					if(count > 1){
						// end/start segment
						std::cout << "[";
						print_coords(location);
						std::cout << "]"; //no trailing comma
						std::cout << "]}}," << std::endl;

						++seg;

						std::cout << "{\"type\":\"Feature\",\"properties\":{\"osm_id\":\"" << way_id << "_" << seg << "\"},";
						std::cout << "\"geometry\":{\"type\":\"LineString\",\"coordinates\":[";
						std::cout << "[";
						print_coords(location);
						std::cout << "],";
					} else {
						// continue segment
						std::cout << "[";
						print_coords(location);
						std::cout << "],";
					}
				}
			}
			++i;
		}

		if(last == 0){
			std::cerr << "One-node way " << way_id << std::endl;
			return;
		}
	}
}

#endif