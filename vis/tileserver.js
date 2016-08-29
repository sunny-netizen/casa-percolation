var express = require('express');
var http = require('http');
var app = express();
var tilelive = require('tilelive');
require('mbtiles').registerProtocols(tilelive);

tilelive.load('mbtiles:///home/tom/vis/data/dbpedia_geo.mbtiles', function(err, dbpedia_geo_tiles) {
tilelive.load('mbtiles:///home/tom/vis/data/dbpedia.mbtiles', function(err, dbpedia_tiles) {
tilelive.load('mbtiles:///home/tom/vis/data/europe_nodes.mbtiles', function(err, europe_nodes_tiles) {
	if (err) {
		throw err;
	}
	app.set('port', 7777);

	app.get('/tiles/:tileset/:z/:x/:y.pbf', function(req, res){
		var tileset = req.params.tileset;
		var z = req.params.z;
		var x = req.params.x;
		var y = req.params.y;

		console.log('get %s tile %d, %d, %d', tileset, z, x, y);

		if(tileset == "dbpedia"){
			dbpedia_tiles.getTile(z, x, y, function(err, tile, headers) {
				if (err) {
					res.status(404);
					res.send(err.message);
					console.log(err.message);
				} else {
					res.set(headers);
					res.send(tile);
				}
			});
		}
		if(tileset == "dbpedia_geo"){
			dbpedia_geo_tiles.getTile(z, x, y, function(err, tile, headers) {
				if (err) {
					res.status(404);
					res.send(err.message);
					console.log(err.message);
				} else {
					res.set(headers);
					res.send(tile);
				}
			});
		}
		if(tileset == "europe_nodes"){
			europe_nodes_tiles.getTile(z, x, y, function(err, tile, headers) {
				if (err) {
					res.status(404);
					res.send(err.message);
					console.log(err.message);
				} else {
					res.set(headers);
					res.send(tile);
				}
			});
		}
	});

	http.createServer(app).listen(app.get('port'), function() {
		console.log('Express server listening on port ' + app.get('port'));
	});
});
});
});
