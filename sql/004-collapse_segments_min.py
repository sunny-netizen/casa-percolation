#!/usr/bin/env python

import sys, psycopg2

# DEBUG = True
DEBUG = False

def main():
	conn = psycopg2.connect("dbname=tom user=tom")
	cur = conn.cursor()
	read_cur = conn.cursor()

	cur.execute("SELECT max(start_node_id) from osm_segments_min")
	start_max = cur.fetchone()
	cur.execute("SELECT max(end_node_id) from osm_segments_min")
	end_max = cur.fetchone()

	upper_limit = max(start_max[0],end_max[0])
	lower_limit = 0
	stride = 10000000

	# loop a stride at a time, avoid materializing a list of ALL start/end points
	scan_all = True
	while scan_all:
		scan_all = False # expect to loop just once, unless some were found
		for nmin in range(lower_limit,upper_limit,stride):
			nmax = nmin+stride+1
			if(DEBUG):
				print "["+str(nmin)+"-"+str(nmax)+"]"

			while True:
				# cur.execute("DELETE FROM osm_segments_min WHERE start_node_id = end_node_id")
				cur.execute("DELETE FROM osm_segments_min WHERE start_node_id = end_node_id and start_node_id > %s and start_node_id < %s", (nmin,nmax))
				conn.commit()

				read_cur.execute(parameterized_order_2_nodes_query_fast, (nmin,nmax,nmin,nmax))

				if (read_cur.rowcount == 0 or read_cur.rowcount == -1 or read_cur.rowcount == None):
					break

				scan_all = True # some were found, so will loop back over whole range

				## Log
				print "["+str(nmin)+"-"+str(nmax)+"] " + str(read_cur.rowcount)

				for node_id in read_cur:
					try:
						merge_on_node(node_id[0],cur)
					except psycopg2.DataError, e:
						# when inserting GeometryCollection, eg from lines that merge to empty
						# log, rollback attempted insert, and just delete lines
						print str(node_id[0]) + ":" + str(e)
						conn.rollback()
						cur.execute("DELETE FROM osm_segments_min WHERE start_node_id = %s or end_node_id = %s", (node_id[0], node_id[0]))
					finally:
						conn.commit()
					if(DEBUG):
						break

				if(DEBUG):
					break
			if(DEBUG):
				break

	cur.close()
	conn.close()

def merge_on_node(node_id,cur):
	cur.execute("""SELECT segment_id, start_node_id, end_node_id, st_astext(geog) as g FROM osm_segments_min WHERE start_node_id = %s OR end_node_id = %s""",(node_id,node_id))
	segs = cur.fetchall()
	if(DEBUG):
		print node_id

	if len(segs) != 2:
		if(DEBUG):
			print "Node "+str(node_id)+" had "+str(len(segs))+" segments"
		return

	endpoints = []
	for sid, start, end, g in segs:
		if(DEBUG):
			print g

		if start == end:
			if(DEBUG):
				print "Segment "+str(sid)+" was "+str(len(segs))+" circular"
			return

		if start != node_id:
			endpoints.append(start)

		if end != node_id:
			endpoints.append(end)

		if(DEBUG):
			print str(start)+"-"+str(end)

	if(DEBUG):
		print endpoints


	if len(endpoints) != 2:
		raise Exception("Node "+str(node_id)+" had "+str(len(endpoints))+" endpoints")


	geog_query = """INSERT INTO osm_segments_min (start_node_id, end_node_id, geog)
	VALUES (%s, %s, (SELECT
			ST_LineMerge(
				ST_Collect(
					geog::geometry
				)
			)
		FROM osm_segments_min
		WHERE start_node_id = %s OR end_node_id = %s)
		);"""
	cur.execute(geog_query, (endpoints[0], endpoints[1], node_id, node_id))
	cur.execute("DELETE FROM osm_segments_min WHERE start_node_id = %s or end_node_id = %s", (node_id, node_id))


### options for getting nodes
get_all_order_2_nodes_query = """SELECT n, count
FROM (
	SELECT n, count(n)
	FROM osm_segments_min,
	LATERAL (VALUES (start_node_id),(end_node_id)) t(n)
	GROUP BY n
) as foo
WHERE count = 2;"""

get_all_order_2_nodes_query_fast = """SELECT id
FROM (
	SELECT id, count(id)
	FROM (
		SELECT start_node_id AS id FROM osm_segments_min
		UNION ALL SELECT end_node_id FROM osm_segments_min
	) as all_ids
	GROUP BY id
) as grouped_ids
WHERE count = 2;"""

parameterized_order_2_nodes_query_fast = """SELECT id
FROM (
	SELECT id, count(id)
	FROM (
		SELECT start_node_id AS id FROM osm_segments_min WHERE start_node_id > %s and start_node_id < %s
		UNION ALL SELECT end_node_id FROM osm_segments_min WHERE end_node_id > %s and end_node_id < %s
	) as all_ids
	GROUP BY id
) as grouped_ids
WHERE count = 2;"""

eg_parameterized_order_2_nodes = """SELECT id
FROM (
	SELECT id, count(id)
	FROM (
		SELECT start_node_id AS id FROM osm_segments_min WHERE start_node_id > 0 and start_node_id < 1000000
		UNION ALL SELECT end_node_id FROM osm_segments_min WHERE end_node_id > 0 and end_node_id < 1000000
	) as all_ids
	GROUP BY id
) as grouped_ids
WHERE count = 2;"""

get_a_segment_with_order_2_ends = """SELECT a.segment_id, a.start_node_id, (
		SELECT count(*)
		FROM osm_segments_min as b
		WHERE a.start_node_id = b.end_node_id OR a.start_node_id = b.start_node_id
	) as start_count,
	a.end_node_id, (
		SELECT count(*)
		FROM osm_segments_min as c
		WHERE a.end_node_id = c.end_node_id OR a.end_node_id = c.start_node_id
	) as end_count
	FROM osm_segments_min as a
	WHERE ((
		SELECT count(*)
		FROM osm_segments_min as b
		WHERE a.start_node_id = b.end_node_id OR a.start_node_id = b.start_node_id
	) = 2 OR (
		SELECT count(*)
		FROM osm_segments_min as c
		WHERE a.end_node_id = c.end_node_id OR a.end_node_id = c.start_node_id
	) = 2)
	ORDER BY segment_id limit 1"""

main()