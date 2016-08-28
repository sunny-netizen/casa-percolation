#include <list>
#include <osmium/osm/location.hpp>
#include <osmium/osm/types.hpp>

// wraps a node: id, location, list of ways that use it, and count of ways that use it
class WayNode {
	// osmium::unsigned_object_id_type m_id;
	osmium::Location m_location;
	int32_t m_count;

	// std::vector<osmium::unsigned_object_id_type> m_ways;

public:
	WayNode():
		// m_id(0),
		m_location(osmium::Location()),
		m_count(0) {
	}

	// WayNode(osmium::unsigned_object_id_type id):
	// 	m_id(id),
	// 	m_location(osmium::Location()),
	// 	m_count(0) {
	// }

	WayNode(osmium::unsigned_object_id_type id, osmium::Location location):
		// m_id(id),
		m_location(location),
		m_count(0) {
	}

	// void add_way(osmium::unsigned_object_id_type id){
	// 	this->m_ways.push_back(id);
	// 	inc();
	// }

	// std::vector<osmium::unsigned_object_id_type> ways() const {
	// 	return m_ways;
	// }

	void inc(){
		++m_count;
	}

	// osmium::unsigned_object_id_type id() const {
	// 	return m_id;
	// }

	int32_t count() const {
		return m_count;
	}

	osmium::Location location() const {
		return m_location;
	}

	// void set_location(osmium::Location location){
	// 	this->m_location = location;
	// }

	/**
     * Locations are equal if both coordinates are equal.
     */
    bool operator==(const WayNode & lhs) const{
        return lhs.location() == m_location;
    }

    bool operator!=(const WayNode & lhs) const{
        return ! (lhs.location() == m_location);
    }

    /**
     * Compare two WayNodes by comparing first the x and then
     * the y coordinate. If either of the WayNodes is
     * undefined the result is undefined.
     */
    bool operator<(const WayNode & lhs) const{
        return lhs.location() < m_location;
    }

    bool operator>(const WayNode & lhs) const{
        return m_location < lhs.location();
    }

    bool operator<=(const WayNode & lhs) const{
        return ! (m_location < lhs.location());
    }

    bool operator>=(const WayNode & lhs) const{
        return ! (lhs.location() < m_location);
    }

	/**
     * Locations are equal if both coordinates are equal.
     */
    // bool operator==(const WayNode & lhs) const{
    //     return lhs.id() == m_id;
    // }

    // bool operator!=(const WayNode & lhs) const{
    //     return ! (lhs.id() == m_id);
    // }

    /**
     * Compare two WayNodes by comparing first the x and then
     * the y coordinate. If either of the WayNodes is
     * undefined the result is undefined.
     */
    // bool operator<(const WayNode & lhs) const{
    //     return lhs.id() < m_id;
    // }

    // bool operator>(const WayNode & lhs) const{
    //     return lhs.id() > m_id;
    // }

    // bool operator<=(const WayNode & lhs) const{
    //     return lhs.id() <= m_id;
    // }

    // bool operator>=(const WayNode & lhs) const{
    //     return lhs.id() >= m_id;
    // }
};