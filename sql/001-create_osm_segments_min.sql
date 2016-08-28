CREATE TABLE osm_segments_min (
	segment_id bigserial,
	start_node_id bigint,
	end_node_id bigint,
	geog geography(LINESTRING,4326)
);