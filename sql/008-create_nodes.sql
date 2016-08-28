CREATE TABLE osm_nodes (
	id bigint,
	degree int,
	min_length int,
	geog geography(POINT,4326)
);
