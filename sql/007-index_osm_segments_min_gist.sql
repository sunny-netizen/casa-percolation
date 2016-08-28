CREATE INDEX idx_osm_segments_min_geog ON osm_segments_min USING GIST ( geog );
VACUUM ANALYZE osm_segments_min ( geog );
CREATE INDEX idx_osm_segments_min_length ON osm_segments_min ( length );
VACUUM ANALYZE osm_segments_min ( length );
