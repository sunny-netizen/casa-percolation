-- Create geographical index later - no need for collapse stage
CREATE UNIQUE INDEX idx_osm_segments_min_id ON osm_segments_min ( segment_id );
CREATE INDEX idx_osm_segments_min_start ON osm_segments_min ( start_node_id );
CREATE INDEX idx_osm_segments_min_end ON osm_segments_min ( end_node_id );
VACUUM ANALYZE osm_segments_min;