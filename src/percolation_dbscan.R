library(dbscan)
library(rgdal)

# Usage: Rscript percolation-dbscan.R data/nodes.tsv 200
options <- commandArgs(trailingOnly = TRUE)
nodes_file <- options[1]
d <- as.integer(options[2])
dtp <- sprintf("%04d",d)

nodes <- read.csv(nodes_file,
                  sep="\t",
                  header=F,
                  col.names=c('id','lon','lat'),
                  colClasses = c("numeric","numeric","numeric"))

# EPSG string
latlon = "+init=epsg:4326"
eulaea = "+init=epsg:3035"
coordinates(nodes) <- c('lon','lat')
proj4string(nodes) <- CRS(latlon)
eu_nodes <- spTransform(nodes, CRS(eulaea))

# calc cluster
res <- dbscan(coordinates(eu_nodes), eps = d, minPts = 2)

# plot cluster
png(filename = paste("point_percolation-",dtp,".png",sep=""),
    width = 1900, height = 1080, units = "px")
plot(eu_nodes, col=res$cluster, pch=".")
dev.off()

nodes$cluster = res$cluster
nodes$optional = NULL
nodes<-nodes[nodes$cluster != 0,]
write.table(nodes, file = paste("point_percolation-",dtp,".tsv",sep=""),row.names=FALSE,col.names=FALSE,sep="\t")

cluster_sizes<-as.data.frame(table(res$cluster))
cluster_sizes$Var1 <- as.integer(cluster_sizes$Var1) - 1
cluster_sizes<-cluster_sizes[cluster_sizes$Var1 != 0,]
write.table(cluster_sizes, file=paste("component_size-",dtp,".txt",sep=""), sep="\t", col.names=FALSE, row.names = F)
