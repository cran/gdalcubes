/*
    MIT License

    Copyright (c) 2019 Marius Appel <marius.appel@uni-muenster.de>

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

// gdalcubes.h shall be used by external programs as library entry point

#ifndef GDALCUBES_H
#define GDALCUBES_H

#include "apply_pixel.h"
#include "build_info.h"
#include "config.h"
#include "cube.h"
#include "dummy.h"
#include "filter_predicate.h"
#include "image_collection_cube.h"
#include "join_bands.h"
#include "progress.h"
#include "reduce.h"
#include "reduce_space.h"
#include "reduce_time.h"
#include "select_bands.h"
#include "stream.h"
#include "swarm.h"
#include "utils.h"
#include "window_time.h"

#endif  //GDALCUBES_H
