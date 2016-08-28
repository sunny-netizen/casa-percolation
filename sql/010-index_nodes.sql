CREATE UNIQUE INDEX idx_osm_nodes_id ON osm_nodes ( id );
CREATE INDEX idx_osm_nodes_length ON osm_nodes ( min_length );
CREATE INDEX idx_osm_nodes_geog ON osm_nodes USING GIST ( geog );
VACUUM ANALYZE osm_nodes;