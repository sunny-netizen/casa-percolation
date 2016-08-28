import sys, csv, json

# read in tsv with nodes as POINT() WKT
# output as line-delimited GeoJSON features
# suitable as input to Tippecanoe

with open('data/europe_nodes_with_coords.tsv','r') as in_file:
	with open('data/europe_nodes.geojson','w') as out_file:
		for line in in_file:
			line = line.replace("POINT(","").replace(")","")
			nid, degree, min_length, lon, lat = line.split(" ")

			feature = {
				"type": "Feature",
				"geometry": {
					"type": "Point",
					"coordinates": [float(lon), float(lat)]
				},
				"properties": {
					"i": int(nid),
					"l": int(min_length)
				}
			}
			out_file.write(json.dumps(feature)+",\n")
#			break

