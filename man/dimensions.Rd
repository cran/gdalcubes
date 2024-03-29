% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/cube.R
\name{dimensions}
\alias{dimensions}
\title{Query data cube properties}
\usage{
dimensions(obj)
}
\arguments{
\item{obj}{a data cube proxy object (class cube)}
}
\value{
Dimension information as a list
}
\description{
Query data cube properties
}
\details{
Elements of the returned list represent individual dimensions with properties such as dimension boundaries, names, and chunk size stored as inner lists
}
\examples{
# create image collection from example Landsat data only 
# if not already done in other examples
if (!file.exists(file.path(tempdir(), "L8.db"))) {
  L8_files <- list.files(system.file("L8NY18", package = "gdalcubes"),
                         ".TIF", recursive = TRUE, full.names = TRUE)
  create_image_collection(L8_files, "L8_L1TP", file.path(tempdir(), "L8.db"), quiet = TRUE) 
}

L8.col = image_collection(file.path(tempdir(), "L8.db"))
v = cube_view(extent=list(left=388941.2, right=766552.4, 
              bottom=4345299, top=4744931, t0="2018-04", t1="2018-06"),
              srs="EPSG:32618", nx = 497, ny=526, dt="P1M")
dimensions(raster_cube(L8.col, v))
}
