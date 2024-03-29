% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/cube.R
\name{image_mask}
\alias{image_mask}
\title{Create a mask for images in a raster data cube}
\usage{
image_mask(
  band,
  min = NULL,
  max = NULL,
  values = NULL,
  bits = NULL,
  invert = FALSE
)
}
\arguments{
\item{band}{name of the mask band}

\item{min}{minimum value, values between \code{min} and \code{max} will be masked}

\item{max}{maximum value, values between \code{min} and \code{max} will be masked}

\item{values}{numeric vector; specific values that will be masked.}

\item{bits}{for bitmasks, extract the given bits (integer vector) with a bitwise AND before filtering the mask values, bit indexes are zero-based}

\item{invert}{logical; invert mask}
}
\description{
Create an image mask based on a band and provided values to filter pixels of images 
read by \code{\link{raster_cube}}
}
\details{
Values of the selected mask band can be based on a range (by passing \code{min} and \code{max}) or on a set of values (by passing \code{values}). By default
pixels with mask values contained in the range or in the values are masked out, i.e. set to NA. Setting \code{invert = TRUE} will invert the masking behavior.
Passing \code{values} will override \code{min} and \code{max}.
}
\note{
Notice that masks are applied per image while reading images as a raster cube. They can be useful to eliminate e.g. cloudy pixels before applying the temporal aggregation to
merge multiple values for the same data cube pixel.
}
\examples{
image_mask("SCL", values = c(3,8,9)) # Sentinel 2 L2A: mask cloud and cloud shadows
image_mask("BQA", bits=4, values=16) # Landsat 8: mask clouds
image_mask("B10", min = 8000, max=65000) 

}
