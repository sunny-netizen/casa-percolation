#!/usr/bin/env python

import time, psycopg2

conn = psycopg2.connect("dbname=tom user=tom")
cur = conn.cursor()

cur.execute("SELECT max(segment_id) from osm_segments_min")
id_max = cur.fetchone()

upper_limit = id_max[0]
print upper_limit

lower_limit = 32250000
stride = 10000

for nmin in range(lower_limit, upper_limit, stride):
	nmax = nmin+stride
	print nmin
	cur.execute("UPDATE osm_segments_min SET length = ST_Length(geog) WHERE segment_id > %s and segment_id <= %s",(nmin,nmax) )
	conn.commit()
	if nmin % (stride*10) == 0:
		time.sleep(10)

cur.close()
conn.close()

