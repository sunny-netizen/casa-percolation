// to parse pbf files
#include <osmium/io/pbf_input.hpp>

// shared print-format functions
#include "print_utils.hpp"

int main (int argc, char* argv[]) {
	try {
		// check arguments, expect a filename
		if (argc != 2) {
			std::cerr << "Usage: " << argv[0] << " filename.osm.pbf\n";
			exit(1);
		}

		// Read from file passed as argument, only nodes and ways
		osmium::io::Reader reader(argv[1], osmium::osm_entity_bits::node | osmium::osm_entity_bits::way);

		// set up numbers to report counts
		int number_of_nodes = 0;
		int number_of_ways = 0;
		int number_of_relations = 0;
		int number_of_others = 0;

		// Parse out header
		osmium::io::Header header = reader.header();
		PBFPrinter::print_header(header);

		// Read data into buffer
		while (osmium::memory::Buffer buffer = reader.read()) {

			// Loop through the OSM entities in the buffer
			for (auto entity = std::begin(buffer), end = std::end(buffer); entity != end; ++entity) {
				switch (entity->type()) {
					case osmium::item_type::node:
						++number_of_nodes;
						break;
					case osmium::item_type::way:
						++number_of_ways;
						break;
					case osmium::item_type::relation:
						++number_of_relations;
						break;
					default:
						++number_of_others;
						break;
				}
			}
		}

		// Print summary and close
		PBFPrinter::print_counts(number_of_nodes,number_of_ways,number_of_relations,number_of_others);
		reader.close();

		return 0;

	} catch(const std::runtime_error& e) {

		fprintf(stderr, "Failed due to runtime error: %s\n", e.what());
		exit(EXIT_FAILURE);

	}
}
