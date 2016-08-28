#include <list>
#include <osmium/osm/location.hpp>

// wraps a location and a count
class WaySegment {
	osmium::unsigned_object_id_type m_way_id;
	int32_t m_segment_number;
	std::list<osmium::NodeRef> m_nodes;

public:
	WaySegment():
		m_way_id(0),
		m_segment_number(0) {
	}

	WaySegment(osmium::unsigned_object_id_type way_id):
		m_way_id(way_id),
		m_segment_number(0) {
	}

	WaySegment(osmium::unsigned_object_id_type way_id, int32_t segment_number):
		m_way_id(way_id),
		m_segment_number(segment_number) {
	}

	void addNode(osmium::NodeRef node){
		m_nodes.push_back(node);
	}

	osmium::unsigned_object_id_type wayId(){
		return m_way_id;
	}

	int32_t segmentNumber(){
		return m_segment_number;
	}

	std::list<osmium::NodeRef> nodes(){
		return m_nodes;
	}

};