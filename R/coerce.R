#' Coerce gdalcubes object into a stars object
#' 
#' The function materializes a data cube as a temporary netCDF file and loads the file 
#' with the stars package.
#' 
#' @param .x data cube object to coerce
#' @param ... not used
#' @return stars object
#' @examples 
#' \donttest{
#' # create image collection from example Landsat data only 
#' # if not already done in other examples
#' if (!file.exists(file.path(tempdir(), "L8.db"))) {
#'   L8_files <- list.files(system.file("L8NY18", package = "gdalcubes"),
#'                          ".TIF", recursive = TRUE, full.names = TRUE)
#'   create_image_collection(L8_files, "L8_L1TP", file.path(tempdir(), "L8.db"), quiet = TRUE) 
#' }
#' 
#' L8.col = image_collection(file.path(tempdir(), "L8.db"))
#' v = cube_view(extent=list(left=388941.2, right=766552.4, 
#'               bottom=4345299, top=4744931, t0="2018-04", t1="2018-04"),
#'               srs="EPSG:32618", nx = 497, ny=526, dt="P1M")
#' if(require("stars"))
#'   st_as_stars(select_bands(raster_cube(L8.col, v), c("B04", "B05")))
#' }
#' @export
st_as_stars.cube <- function(.x, ...) { 
  
  outnc = tempfile(fileext = ".nc")
  #subdatasets = paste0("NETCDF:\"", outnc, "\":", names(from), sep="", collapse = NULL)
  if ("ncdf_cube" %in% class(.x)) {
    outnc = jsonlite::parse_json(as_json(.x))$file
    if (is.null(outnc)) {
      stop("Invalid ncdf cube; missing file reference")
    }
  }
  else {
    write_ncdf(.x, outnc)
  }
  sds = paste0("NETCDF:\"", outnc, "\":", names(.x))
  out = stars::read_stars(sds)
  #  
  #  attr(out, "dimensions")$x$offset = dimensions(from)$x$low
  #  attr(out, "dimensions")$x$delta  = dimensions(from)$x$pixel_size
  #  attr(out, "dimensions")$x["values"] = list(NULL)
  #  #attr(out, "dimensions")$x$point = FALSE
  #  
  #  attr(out, "dimensions")$y$offset = dimensions(from)$y$high
  #  attr(out, "dimensions")$y$delta  = -dimensions(from)$y$pixel_size
  #  attr(out, "dimensions")$y["values"]  = list(NULL)
  #  #attr(out, "dimensions")$y$point = FALSE
  #  
  attr(out, "dimensions")$time$point = FALSE
  attr(out, "dimensions")$time$offset = NA
  attr(out, "dimensions")$time$delta = NA
  pst = dimensions(.x)$t$pixel_size
  if (endsWith(pst, "Y") || endsWith(pst, "M") || endsWith(pst, "D")) {
    tt = dimension_bounds(.x, "d")$t
    #attr(out, "dimensions")$time$values = as.Date(dimension_values(.x, "d")$t)
    values = list(start = as.Date(tt$start), end = as.Date(tt$end))
    class(values) <- "intervals"
    attr(out, "dimensions")$time$values = values
  }
 else {
   tt = dimension_bounds(.x, "S")$t
   #attr(out, "dimensions")$time$values = as.POSIXct(dimension_values(.x)$t)
   values = list(start = as.Date(tt$start), end = as.Date(tt$end))
   class(values) <- "intervals"
   attr(out, "dimensions")$time$values = values
 }
  return(out)
}


#' Convert a data cube to an in-memory R array
#' 
#' @param x data cube
#' @return Four dimensional array with dimensions band, t, y, x
#' @examples 
#' \donttest{
#' # create image collection from example Landsat data only 
#' # if not already done in other examples
#' if (!file.exists(file.path(tempdir(), "L8.db"))) {
#'   L8_files <- list.files(system.file("L8NY18", package = "gdalcubes"),
#'                          ".TIF", recursive = TRUE, full.names = TRUE)
#'   create_image_collection(L8_files, "L8_L1TP", file.path(tempdir(), "L8.db"), quiet = TRUE) 
#' }
#' 
#' L8.col = image_collection(file.path(tempdir(), "L8.db"))
#' v = cube_view(extent=list(left=388941.2, right=766552.4, 
#'               bottom=4345299, top=4744931, t0="2018-04", t1="2018-05"),
#'               srs="EPSG:32618", nx = 100, ny=100, dt="P1M")
#' x = as_array(select_bands(raster_cube(L8.col, v), c("B04", "B05")))
#' dim(x)
#' dimnames(x)
#' }
#' @note Depending on the data cube size, this function may require substantial amounts of main memory, i.e.
#' it makes sense for small data cubes only.
#' @export
as_array <- function(x) {
  
  stopifnot(is.cube(x))
  size = c(nbands(x), size(x))
  
  if ("ncdf_cube" %in% class(x)) {
    fn = jsonlite::parse_json(as_json(x))$file
    if (is.null(fn)) {
      stop("Invalid ncdf cube; missing file reference")
    }
  }
  else {
    fn = tempfile(fileext = ".nc")
    write_ncdf(x, fn)
  }
  
  f <- ncdf4::nc_open(fn)
  # derive name of variables but ignore non three-dimensional variables (e.g. crs)
  vars <- names(which(sapply(f$var, function(v) {
    if (v$ndims == 3)
      return(v$name)
    return("")
  }) != ""))
  
  out = array(NA, dim=size)
  for (b in 1:length(vars)) {
    z = aperm(ncdf4::ncvar_get(f, vars[b], collapse_degen=FALSE), c(3,2,1))  # does it drop dims?
    dim(z) <- size[2:4]
    out[b,,,] = z
  }
  ncdf4::nc_close(f)
  
  dv <- dimension_values(x)
  dimnames(out) <- list(bands=vars, t=dv$t, y=dv$y, x=dv$x)
  
  return(out)
}





#' Convert a data cube to a data.frame
#' 
#' @param x data cube object
#' @param ... not used
#' @param complete_only logical; if TRUE, remove rows with one or more missing values
#' @return A data.frame with bands / variables of the cube as columns and pixels as rows
#' @examples 
#' \donttest{
#' # create image collection from example Landsat data only 
#' # if not already done in other examples
#' if (!file.exists(file.path(tempdir(), "L8.db"))) {
#'   L8_files <- list.files(system.file("L8NY18", package = "gdalcubes"),
#'                          ".TIF", recursive = TRUE, full.names = TRUE)
#'   create_image_collection(L8_files, "L8_L1TP", file.path(tempdir(), "L8.db"), quiet = TRUE) 
#' }
#' 
#' L8.col = image_collection(file.path(tempdir(), "L8.db"))
#' v = cube_view(extent=list(left=388941.2, right=766552.4, 
#'               bottom=4345299, top=4744931, t0="2018-04", t1="2018-05"),
#'               srs="EPSG:32618", nx = 100, ny=100, dt="P1M")
#' x = select_bands(raster_cube(L8.col, v), c("B04", "B05"))
#' df = as.data.frame(x, complete_only = TRUE)
#' head(df)
#' }
#' @export
as.data.frame.cube<- function(x, ..., complete_only = FALSE) {
  stopifnot(is.cube(x))
  d = c(nbands(x), dim(x))
  nb=d[1]
  nobs = prod(d[2:4])
  
  if ("ncdf_cube" %in% class(x)) {
    fn = jsonlite::parse_json(as_json(x))$file
    if (is.null(fn)) {
      stop("Invalid ncdf cube; missing file reference")
    }
  }
  else {
    fn = tempfile(fileext = ".nc")
    write_ncdf(x, fn)
  }
  
  f <- ncdf4::nc_open(fn)
  # derive name of variables but ignore non three-dimensional variables (e.g. crs)
  vars <- names(which(sapply(f$var, function(v) {
    if (v$ndims == 3)
      return(v$name)
    return("")
  }) != ""))
  
  out = data.frame(matrix(numeric(nobs*nb), nrow=nobs, ncol=nb))
  colnames(out) = vars
  
  for (b in 1:length(vars)) {
    out[b] = as.vector(ncdf4::ncvar_get(f, vars[b])) #, collapse_degen=FALSE)
  }
  ncdf4::nc_close(f)
  
  if (complete_only) {
    out = out[complete.cases(out),]
  }
  return(out)
}

