library(sp)
library(raster)
library(rgdal)

# clip raster to broad Europe bbox

landwater = raster("data/landlake.asc") # from Hyde 3.1
plot(landwater)

europe_bbox = readOGR("data/europe.kml",layer="europe") # from download.geofabrik.de
plot(europe_bbox)

europe_landwater = mask(landwater, europe_bbox)
plot(europe_landwater)

writeRaster(europe_landwater, "data/europe_landwater.asc", "ascii")
extent(europe_bbox)
