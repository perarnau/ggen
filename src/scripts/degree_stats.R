#!/usr/bin/Rscript
#################################################
# This script takes the out_degree output
# of graph_analysis and computes :
#	min, max and mean of the out_degrees
#	the histogram of degrees
# This is not part of GGen, and we do not guarranty
# that it works.
# Copyright Swann Perarnau, 2009
#################################################
require("graphics")
# read the values
data <- read.table("stdin",header=TRUE)

# if all went well, we now have a two column table
# with for each vertex its out_degree

# min, max, mean, etc
print("Statistics:",quote=FALSE)
dmin <- min(data[,2])
paste("Min:",dmin)
dmax <- max(data[,2])
paste("Max:",dmax)
paste("Mean:",mean(data[,2]))
paste("Variance:",var(data[,2]))
paste("Standard Dev.:",sd(data[,2]))

# open the ps device
print("Printing the data histogram:",quote=FALSE)
setEPS()
postscript("hist.eps")
# create histogram
b <- dmax - dmin + 1
hist(data[,2],breaks=b)
#close file
invisible(dev.off())
print("Everything done, quitting...",quote=FALSE)
