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

#include "cube.h"
#include <thread>

#include <netcdf.h>
#include "build_info.h"
#include "filesystem.h"

void cube::write_gtiff_directory(std::string dir, std::shared_ptr<chunk_processor> p) {
    if (!filesystem::exists(dir)) {
        filesystem::mkdir_recursive(dir);
    }

    if (!filesystem::is_directory(dir)) {
        throw std::string("ERROR in chunking::write_gtiff_directory(): output is not a directory.");
    }

    GDALDriver *gtiff_driver = (GDALDriver *)GDALGetDriverByName("GTiff");
    if (gtiff_driver == NULL) {
        throw std::string("ERROR: cannot find GDAL driver for GTiff.");
    }

    std::shared_ptr<progress> prg = config::instance()->get_default_progress_bar()->get();
    prg->set(0);  // explicitly set to zero to show progress bar immediately

    std::function<void(chunkid_t, std::shared_ptr<chunk_data>, std::mutex &)> f = [this, dir, prg, gtiff_driver](chunkid_t id, std::shared_ptr<chunk_data> dat, std::mutex &m) {
        bounds_st cextent = this->bounds_from_chunk(id);  // implemented in derived classes
        double affine[6];
        affine[0] = cextent.s.left;
        affine[3] = cextent.s.top;
        affine[1] = _st_ref->dx();
        affine[5] = -_st_ref->dy();
        affine[2] = 0.0;
        affine[4] = 0.0;

        CPLStringList out_co;
        //out_co.AddNameValue("TILED", "YES");
        //out_co.AddNameValue("BLOCKXSIZE", "256");
        // out_co.AddNameValue("BLOCKYSIZE", "256");

        for (uint16_t ib = 0; ib < dat->count_bands(); ++ib) {
            for (uint32_t it = 0; it < dat->size()[1]; ++it) {
                std::string out_file = filesystem::join(dir, (std::to_string(id) + "_" + std::to_string(ib) + "_" + std::to_string(it) + ".tif"));

                GDALDataset *gdal_out = gtiff_driver->Create(out_file.c_str(), dat->size()[3], dat->size()[2], 1, GDT_Float64, out_co.List());
                CPLErr res = gdal_out->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, dat->size()[3], dat->size()[2], ((double *)dat->buf()) + (ib * dat->size()[1] * dat->size()[2] * dat->size()[3] + it * dat->size()[2] * dat->size()[3]), dat->size()[3], dat->size()[2], GDT_Float64, 0, 0, NULL);
                if (res != CE_None) {
                    GCBS_WARN("RasterIO (write) failed for band " + _bands.get(ib).name);
                }
                gdal_out->GetRasterBand(1)->SetNoDataValue(std::stod(_bands.get(ib).no_data_value));
                char *wkt_out;
                OGRSpatialReference srs_out;
                srs_out.SetFromUserInput(_st_ref->srs().c_str());
                srs_out.exportToWkt(&wkt_out);

                GDALSetProjection(gdal_out, wkt_out);
                GDALSetGeoTransform(gdal_out, affine);
                CPLFree(wkt_out);

                GDALClose(gdal_out);
            }
        }
        prg->increment((double)1 / (double)this->count_chunks());
    };

    p->apply(shared_from_this(), f);
    prg->finalize();
}

void cube::write_netcdf_file(std::string path, uint8_t compression_level, std::shared_ptr<chunk_processor> p) {
    std::string op = filesystem::make_absolute(path);

    if (filesystem::is_directory(op)) {
        throw std::string("ERROR in cube::write_netcdf_file(): output already exists and is a directory.");
    }
    if (filesystem::is_regular_file(op)) {
        GCBS_INFO("Existing file '" + op + "' will be overwritten for NetCDF export");
    }

    if (!filesystem::exists(filesystem::parent(op))) {
        filesystem::mkdir_recursive(filesystem::parent(op));
    }

    std::shared_ptr<progress> prg = config::instance()->get_default_progress_bar()->get();
    prg->set(0);  // explicitly set to zero to show progress bar immediately

    double *dim_x = (double *)std::calloc(size_x(), sizeof(double));  //TODO: check for std::free()
    double *dim_y = (double *)std::calloc(size_y(), sizeof(double));
    int *dim_t = (int *)std::calloc(size_t(), sizeof(int));

    double *dim_x_bnds = (double *)std::calloc(size_x() * 2, sizeof(double));  //TODO: check for std::free()
    double *dim_y_bnds = (double *)std::calloc(size_y() * 2, sizeof(double));
    int *dim_t_bnds = (int *)std::calloc(size_t() * 2, sizeof(int));

    if (_st_ref->dt().dt_unit == datetime_unit::WEEK) {
        _st_ref->dt_unit() = datetime_unit::DAY;
        _st_ref->dt_interval() *= 7;  // UDUNIT does not support week
    }
    for (uint32_t i = 0; i < size_t(); ++i) {
        dim_t[i] = (i * st_reference()->dt().dt_interval);
        dim_t_bnds[2 * i] = (i * st_reference()->dt().dt_interval);
        dim_t_bnds[2 * i + 1] = ((i + 1) * st_reference()->dt().dt_interval);
    }
    for (uint32_t i = 0; i < size_y(); ++i) {
        dim_y[i] = st_reference()->win().bottom + size_y() * st_reference()->dy() - (i + 0.5) * st_reference()->dy();  // cell center
        dim_y_bnds[2 * i] = st_reference()->win().bottom + size_y() * st_reference()->dy() - (i)*st_reference()->dy();
        dim_y_bnds[2 * i + 1] = st_reference()->win().bottom + size_y() * st_reference()->dy() - (i + 1) * st_reference()->dy();
    }
    for (uint32_t i = 0; i < size_x(); ++i) {
        dim_x[i] = st_reference()->win().left + (i + 0.5) * st_reference()->dx();
        dim_x_bnds[2 * i] = st_reference()->win().left + (i + 0) * st_reference()->dx();
        dim_x_bnds[2 * i + 1] = st_reference()->win().left + (i + 1) * st_reference()->dx();
    }

    OGRSpatialReference srs = st_reference()->srs_ogr();
    std::string yname = srs.IsProjected() ? "y" : "latitude";
    std::string xname = srs.IsProjected() ? "x" : "longitude";

    int ncout;
    nc_create(op.c_str(), NC_NETCDF4, &ncout);

    int d_t, d_y, d_x;
    nc_def_dim(ncout, "time", size_t(), &d_t);
    nc_def_dim(ncout, yname.c_str(), size_y(), &d_y);
    nc_def_dim(ncout, xname.c_str(), size_x(), &d_x);

    int d_bnds;
    nc_def_dim(ncout, "nv", 2, &d_bnds);

    int v_t, v_y, v_x;
    nc_def_var(ncout, "time", NC_INT, 1, &d_t, &v_t);
    nc_def_var(ncout, yname.c_str(), NC_DOUBLE, 1, &d_y, &v_y);
    nc_def_var(ncout, xname.c_str(), NC_DOUBLE, 1, &d_x, &v_x);

    int v_tbnds, v_ybnds, v_xbnds;
    int d_tbnds[] = {d_t, d_bnds};
    int d_ybnds[] = {d_y, d_bnds};
    int d_xbnds[] = {d_x, d_bnds};
    nc_def_var(ncout, "time_bnds", NC_INT, 2, d_tbnds, &v_tbnds);
    nc_def_var(ncout, "y_bnds", NC_DOUBLE, 2, d_ybnds, &v_ybnds);
    nc_def_var(ncout, "x_bnds", NC_DOUBLE, 2, d_xbnds, &v_xbnds);

    std::string att_source = "gdalcubes " + std::to_string(GDALCUBES_VERSION_MAJOR) + "." + std::to_string(GDALCUBES_VERSION_MINOR) + "." + std::to_string(GDALCUBES_VERSION_PATCH);

    nc_put_att_text(ncout, NC_GLOBAL, "Conventions", strlen("CF-1.6"), "CF-1.6");
    nc_put_att_text(ncout, NC_GLOBAL, "source", strlen(att_source.c_str()), att_source.c_str());

    // write json graph as metadata
    //    std::string j = make_constructible_json().dump();
    //    nc_put_att_text(ncout, NC_GLOBAL, "process_graph", j.length(), j.c_str());

    char *wkt;
    srs.exportToWkt(&wkt);

    double geoloc_array[6] = {st_reference()->left(), st_reference()->dx(), 0.0, st_reference()->top(), 0.0, st_reference()->dy()};
    nc_put_att_text(ncout, NC_GLOBAL, "spatial_ref", strlen(wkt), wkt);
    nc_put_att_double(ncout, NC_GLOBAL, "GeoTransform", NC_DOUBLE, 6, geoloc_array);

    std::string dtunit_str;
    if (_st_ref->dt().dt_unit == datetime_unit::YEAR) {
        dtunit_str = "years";  // WARNING: UDUNITS defines a year as 365.2425 days
    } else if (_st_ref->dt().dt_unit == datetime_unit::MONTH) {
        dtunit_str = "months";  // WARNING: UDUNITS defines a month as 1/12 year
    } else if (_st_ref->dt().dt_unit == datetime_unit::DAY) {
        dtunit_str = "days";
    } else if (_st_ref->dt().dt_unit == datetime_unit::HOUR) {
        dtunit_str = "hours";
    } else if (_st_ref->dt().dt_unit == datetime_unit::MINUTE) {
        dtunit_str = "minutes";
    } else if (_st_ref->dt().dt_unit == datetime_unit::SECOND) {
        dtunit_str = "seconds";
    }
    dtunit_str += " since ";
    dtunit_str += _st_ref->t0().to_string(datetime_unit::SECOND);

    nc_put_att_text(ncout, v_t, "units", strlen(dtunit_str.c_str()), dtunit_str.c_str());
    nc_put_att_text(ncout, v_t, "calendar", strlen("gregorian"), "gregorian");
    nc_put_att_text(ncout, v_t, "long_name", strlen("time"), "time");
    nc_put_att_text(ncout, v_t, "standard_name", strlen("time"), "time");

    if (srs.IsProjected()) {
        char *unit = nullptr;
        srs.GetLinearUnits(&unit);

        nc_put_att_text(ncout, v_y, "units", strlen(unit), unit);
        nc_put_att_text(ncout, v_x, "units", strlen(unit), unit);

        int v_crs;
        nc_def_var(ncout, "crs", NC_INT, 0, NULL, &v_crs);
        nc_put_att_text(ncout, v_crs, "grid_mapping_name", strlen("easting_northing"), "easting_northing");
        nc_put_att_text(ncout, v_crs, "crs_wkt", strlen(wkt), wkt);

    } else {
        // char* unit;
        // double scale = srs.GetAngularUnits(&unit);
        nc_put_att_text(ncout, v_y, "units", strlen("degrees_north"), "degrees_north");
        nc_put_att_text(ncout, v_y, "long_name", strlen("latitude"), "latitude");
        nc_put_att_text(ncout, v_y, "standard_name", strlen("latitude"), "latitude");

        nc_put_att_text(ncout, v_x, "units", strlen("degrees_east"), "degrees_east");
        nc_put_att_text(ncout, v_x, "long_name", strlen("longitude"), "longitude");
        nc_put_att_text(ncout, v_x, "standard_name", strlen("longitude"), "longitude");

        int v_crs;
        nc_def_var(ncout, "crs", NC_INT, 0, NULL, &v_crs);
        nc_put_att_text(ncout, v_crs, "grid_mapping_name", strlen("latitude_longitude"), "latitude_longitude");
        nc_put_att_text(ncout, v_crs, "crs_wkt", strlen(wkt), wkt);
    }
    CPLFree(wkt);
    int d_all[] = {d_t, d_y, d_x};

    std::vector<int> v_bands;

    for (uint16_t i = 0; i < bands().count(); ++i) {
        int v;
        nc_def_var(ncout, bands().get(i).name.c_str(), NC_DOUBLE, 3, d_all, &v);
        std::size_t csize[3] = {_chunk_size[0], _chunk_size[1], _chunk_size[2]};
        nc_def_var_chunking(ncout, v, NC_CHUNKED, csize);
        if (compression_level > 0) {
            nc_def_var_deflate(ncout, v, 1, 1, compression_level);  // TODO: experiment with shuffling
        }

        if (!bands().get(i).unit.empty())
            nc_put_att_text(ncout, v, "units", strlen(bands().get(i).unit.c_str()), bands().get(i).unit.c_str());

        double pscale = bands().get(i).scale;
        double poff = bands().get(i).offset;
        nc_put_att_double(ncout, v, "scale_factor", NC_DOUBLE, 1, &pscale);
        nc_put_att_double(ncout, v, "add_offset", NC_DOUBLE, 1, &poff);
        nc_put_att_text(ncout, v, "type", strlen(bands().get(i).type.c_str()), bands().get(i).type.c_str());
        nc_put_att_text(ncout, v, "grid_mapping", strlen("crs"), "crs");

        // this doesn't seem to solve missing spatial reference for multitemporal nc files
        //        nc_put_att_text(ncout, v, "spatial_ref", strlen(wkt), wkt);
        //        nc_put_att_double(ncout, v, "GeoTransform", NC_DOUBLE, 6, geoloc_array);

        double pNAN = NAN;
        nc_put_att_double(ncout, v, "_FillValue", NC_DOUBLE, 1, &pNAN);

        v_bands.push_back(v);
    }

    nc_enddef(ncout);  ////////////////////////////////////////////////////

    nc_put_var(ncout, v_t, (void *)dim_t);
    nc_put_var(ncout, v_y, (void *)dim_y);
    nc_put_var(ncout, v_x, (void *)dim_x);

    nc_put_var(ncout, v_tbnds, (void *)dim_t_bnds);
    nc_put_var(ncout, v_ybnds, (void *)dim_y_bnds);
    nc_put_var(ncout, v_xbnds, (void *)dim_x_bnds);

    if (dim_t) std::free(dim_t);
    if (dim_y) std::free(dim_y);
    if (dim_x) std::free(dim_x);
    if (dim_t_bnds) std::free(dim_t_bnds);
    if (dim_y_bnds) std::free(dim_y_bnds);
    if (dim_x_bnds) std::free(dim_x_bnds);

    std::function<void(chunkid_t, std::shared_ptr<chunk_data>, std::mutex &)> f = [this, op, prg, &v_bands, ncout](chunkid_t id, std::shared_ptr<chunk_data> dat, std::mutex &m) {
        chunk_size_btyx csize = dat->size();
        bounds_nd<uint32_t, 3> climits = chunk_limits(id);

        //std::vector<std::size_t> startp = {climits.low[0], climits.low[1], climits.low[2]};
        std::size_t startp[] = {climits.low[0], size_y() - climits.high[1] - 1, climits.low[2]};

        std::size_t countp[] = {csize[1], csize[2], csize[3]};

        for (uint16_t i = 0; i < bands().count(); ++i) {
            m.lock();
            nc_put_vara(ncout, v_bands[i], startp, countp, (void *)(((double *)dat->buf()) + (int)i * (int)csize[1] * (int)csize[2] * (int)csize[3]));
            m.unlock();
        }
        prg->increment((double)1 / (double)this->count_chunks());
    };

    p->apply(shared_from_this(), f);
    nc_close(ncout);
    prg->finalize();
}

void chunk_processor_singlethread::apply(std::shared_ptr<cube> c,
                                         std::function<void(chunkid_t, std::shared_ptr<chunk_data>, std::mutex &)> f) {
    std::mutex mutex;
    uint32_t nchunks = c->count_chunks();
    for (uint32_t i = 0; i < nchunks; ++i) {
        std::shared_ptr<chunk_data> dat = c->read_chunk(i);
        f(i, dat, mutex);
    }
}

void chunk_processor_multithread::apply(std::shared_ptr<cube> c,
                                        std::function<void(chunkid_t, std::shared_ptr<chunk_data>, std::mutex &)> f) {
    std::mutex mutex;
    std::vector<std::thread> workers;
    for (uint16_t it = 0; it < _nthreads; ++it) {
        workers.push_back(std::thread([this, &c, f, it, &mutex](void) {
            for (uint32_t i = it; i < c->count_chunks(); i += _nthreads) {
                try {
                    std::shared_ptr<chunk_data> dat = c->read_chunk(i);
                    f(i, dat, mutex);
                } catch (std::string s) {
                    GCBS_ERROR(s);
                    continue;
                } catch (...) {
                    GCBS_ERROR("unexpected exception while processing chunk " + std::to_string(i));
                    continue;
                }
            }
        }));
    }
    for (uint16_t it = 0; it < _nthreads; ++it) {
        workers[it].join();
    }
}
