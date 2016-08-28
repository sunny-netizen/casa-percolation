// to parse pbf files
#include <osmium/io/pbf_input.hpp>
#include <osmium/geom/wkt.hpp>
#include "print_utils.hpp"
#include <unordered_set>

typedef std::unordered_set<osmium::object_id_type> node_ids_collection;

int main (int argc, char* argv[]) {
	try {
		// check arguments, expect a filename
		if (argc != 3) {
			std::cerr << "Usage: " << argv[0] << " data.osm.pbf node_ids.tsv\n";
			exit(1);
		}

		node_ids_collection node_ids;
		std::ifstream input(argv[2]);
		for (std::string id; std::getline(input, id); ) {
			osmium::object_id_type node_id = static_cast<osmium::object_id_type>(stoll(id));
			node_ids.insert(node_id);
			// break;
		}

		// Read from file passed as argument, only nodes and ways
		osmium::io::Reader reader(argv[1], osmium::osm_entity_bits::node);

		// set up factory to write well-known-text
		osmium::geom::WKTFactory<> factory;

		// Read data into buffer
		while (osmium::memory::Buffer buffer = reader.read()) {
			// Loop through the OSM entities in the buffer
			for (auto entity = std::begin(buffer), end = std::end(buffer); entity != end; ++entity) {
				osmium::Node & node = static_cast<osmium::Node &>(*entity);
				auto search = node_ids.find(node.id());
				if (search != node_ids.end()){
					std::string wkt = factory.create_point(node);
					std::cout << node.id() << "\t" << wkt << std::endl;
				}
			}
		}

		// Close
		reader.close();

		return 0;

	} catch(const std::runtime_error& e) {

		fprintf(stderr, "Failed due to runtime error: %s\n", e.what());
		exit(EXIT_FAILURE);

	}
}
