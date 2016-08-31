library(sp)         # spatial utilities
library(raster)     # read rasters

# land outline
europe_landwater = raster("data/europe_landwater.asc")
europe_dem = raster("data/europe-srtm-sampled.tif")

# EPSG strings
latlong = "+init=epsg:4326"
eulaea = "+init=epsg:3035"

get_random_coords <- function(n){
  # x is uniform sample from longitude segment
  x = runif(n,-34.49296,46.75348)

  # y is uniform sample from portion of [0,1) that relates to europe bbox,
  # then converted back to degrees latitude
  # ymin sin(29.73514*pi/180)
  # ymax sin(81.47299*pi/180)
  # using sine distribution for sphere point picking
  # cf: http://mathworld.wolfram.com/SpherePointPicking.html
  # most at equator, few at pole. vis:
  # qplot(vapply(runif(10000,0,1), function(x) asin(x)*180/pi, 0.0))
  y = vapply(runif(n,0.4959913,0.9889461), function(i) asin(i)*180/pi, 0.0)

  SpatialPoints(data.frame(x,y), proj4string = CRS(latlong))
}

get_land_altitude_coords <- function(n){
  out = data.frame()
  while(nrow(out)<n){
    ncoords = get_random_coords(n/2) # try getting n/2 random; some will fail

    # extract whether points are on land
    is_land = extract(europe_landwater,ncoords)

    # extract altitude of points
    alt = extract(europe_dem,ncoords)

    ncoords = as.data.frame(SpatialPointsDataFrame(ncoords,data.frame(is_land, alt)))

    # generate random values as threshold altitude
    ncoords$rand_alt = runif(n/2,0,3500)

    # filter
    ncoords = ncoords[complete.cases(ncoords),]
    ncoords = ncoords[ncoords$is_land == 1, ]
    ncoords = ncoords[ncoords$alt <= ncoords$rand_alt, ]

    out = rbind(out,ncoords[,3:4])
    print(paste("Got rows: ", nrow(out)))
    gc()
  }
  out[0:n,] # discard beyond n rows
}


# random points
npoints = 6000000
# npoints = 600
spc_ll <- get_land_altitude_coords(npoints)
write.table(spc_ll,file="altsim.tsv", sep="\t", row.names=F, col.names=F)
