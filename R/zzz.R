.pkgenv <- new.env(parent=emptyenv())

.onLoad <- function(libname,pkgname) {

  # call gdalcubes_init()
  #if(!Sys.getenv("GDALCUBES_STREAMING") == "1") {
  gc_init()
  gc_add_format_dir(file.path(system.file(package="gdalcubes"),"formats")) # add collection formats directory 
  #}
  
  .pkgenv$compression_level = 1
  .pkgenv$cube_cache = new.env()
  .pkgenv$use_cube_cache = TRUE
  .pkgenv$parallel = 1
  .pkgenv$debug = FALSE
  .pkgenv$log_file = ""
  .pkgenv$ncdf_write_bounds = TRUE 
  .pkgenv$use_overview_images = TRUE
  .pkgenv$worker.debug = FALSE
  .pkgenv$worker.compression_level = 0
  .pkgenv$worker.use_overview_images = TRUE
  .pkgenv$worker.gdal_options = list()
  
  if (interactive()) {
    .pkgenv$show_progress = TRUE
  }
  else {
    .pkgenv$show_progress = FALSE
  }
  gc_set_progress(.pkgenv$show_progress)
  .pkgenv$default_chunksize = .default_chunk_size
  
  worker_script = system.file("scripts/worker.R", package = "gdalcubes")
  cmd <- paste(file.path(R.home("bin"),"Rscript"), " --no-save --no-restore \"", worker_script, "\"", sep="")
  .pkgenv$worker.cmd = cmd
  gc_set_process_execution(.pkgenv$parallel, .pkgenv$worker.cmd, .pkgenv$worker.debug, .pkgenv$worker.compression_level, 
                           .pkgenv$worker.use_overview_images, .pkgenv$worker.gdal_options)
  
  .pkgenv$streaming_dir = tempdir()
  gc_set_streamining_dir(.pkgenv$streaming_dir)

  #.pkgenv$swarm = NULL
  register_s3_method("stars","st_as_stars", "cube")
  
  # for windows, rwinlib includes GDAL data and PROJ data in the package and we must set the environment variables
  # PROJ_LIB and GDAL_DATA to make sure GDAL finds the data at the package location
  if (dir.exists(system.file("proj", package="gdalcubes"))) {
    Sys.setenv("PROJ_LIB" = system.file("proj", package="gdalcubes"))
  }
  if (dir.exists(system.file("gdal", package="gdalcubes"))) {
    Sys.setenv("GDAL_DATA" = system.file("gdal", package="gdalcubes"))
  }
  gdalcubes_set_gdal_config("CPL_TMPDIR", tempdir())
  invisible()
}

.onUnload <- function(libpath) {
  #if(!Sys.getenv("GDALCUBES_STREAMING") == "1") {
    gc_cleanup()
  #}
}


.is_streaming <- function() {
  return(Sys.getenv("GDALCUBES_STREAMING") == "1")
}

.onAttach <- function(libname,pkgname)
{
  # if the package is used inside streaming, redirect stdout to stderr
  # in order to not disturb the (binary) communication with gdalcubes
  if(Sys.getenv("GDALCUBES_STREAMING") == "1") {
    sink(stderr())
  }
  #else {
  #  x = gc_version()
  #  #packageStartupMessage(paste("Using gdalcubes library version ", x$VERSION_MAJOR, ".", x$VERSION_MINOR, ".", x$VERSION_PATCH, sep=""))
  #}
}


# From https://github.com/tidyverse/hms/blob/a24d877fb439486f9e3fb5fab35e178aecaa858d/R/zzz.R (accessed 2020-06-03)
register_s3_method <- function(pkg, generic, class, fun = NULL) {
  stopifnot(is.character(pkg), length(pkg) == 1)
  stopifnot(is.character(generic), length(generic) == 1)
  stopifnot(is.character(class), length(class) == 1)
  
  if (is.null(fun)) {
    fun <- get(paste0(generic, ".", class), envir = parent.frame())
  } else {
    stopifnot(is.function(fun))
  }
  
  if (pkg %in% loadedNamespaces()) {
    registerS3method(generic, class, fun, envir = asNamespace(pkg))
  }
  
  # Always register hook in case package is later unloaded & reloaded
  setHook(
    packageEvent(pkg, "onLoad"),
    function(...) {
      registerS3method(generic, class, fun, envir = asNamespace(pkg))
    }
  )
}

