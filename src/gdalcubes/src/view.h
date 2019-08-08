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

#ifndef VIEW_H
#define VIEW_H

#include <algorithm>
#include "datetime.h"
#include "external/json.hpp"
#include "image_collection.h"

namespace gdalcubes {

/**
 * A utility structure to work with different aggregation
 * algorithms
 */
struct aggregation {
    enum class aggregation_type {
        AGG_NONE,
        AGG_MIN,
        AGG_MAX,
        AGG_MEAN,
        AGG_MEDIAN,
        AGG_FIRST,
        AGG_LAST,
        AGG_IMAGE_COUNT,
        AGG_VALUE_COUNT
    };

    static aggregation_type from_string(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (s == "none") {
            return aggregation_type::AGG_NONE;
        } else if (s == "min") {
            return aggregation_type::AGG_MIN;
        } else if (s == "max") {
            return aggregation_type::AGG_MAX;
        } else if (s == "mean") {
            return aggregation_type::AGG_MEAN;
        } else if (s == "median") {
            return aggregation_type::AGG_MEDIAN;
        } else if (s == "first") {
            return aggregation_type::AGG_FIRST;
        } else if (s == "last") {
            return aggregation_type::AGG_LAST;
        } else if (s == "count_images") {
            return aggregation_type::AGG_IMAGE_COUNT;
        } else if (s == "count_values") {
            return aggregation_type::AGG_VALUE_COUNT;
        }
        return aggregation_type::AGG_NONE;
    }

    static std::string to_string(aggregation_type a) {
        switch (a) {
            case aggregation_type::AGG_NONE:
                return "none";
            case aggregation_type::AGG_MIN:
                return "min";
            case aggregation_type::AGG_MAX:
                return "max";
            case aggregation_type::AGG_MEAN:
                return "mean";
            case aggregation_type::AGG_MEDIAN:
                return "median";
            case aggregation_type::AGG_FIRST:
                return "first";
            case aggregation_type::AGG_LAST:
                return "last";
            case aggregation_type::AGG_IMAGE_COUNT:
                return "count_images";
            case aggregation_type::AGG_VALUE_COUNT:
                return "count_values";
            default:
                return "none";
        }
    }
};

/**
 * @brief Utility structure to work with different resampling
 * algorithms and their different types in GDAL
 */
struct resampling {
    /**
         * @brief An enumeration listing all available resampling types
         */
    enum class resampling_type {
        RSMPL_NEAR,
        RSMPL_BILINEAR,
        RSMPL_CUBIC,
        RSMPL_CUBICSPLINE,
        RSMPL_LANCZOS,
        RSMPL_AVERAGE,
        RSMPL_MODE,
        RSMPL_MAX,
        RSMPL_MIN,
        RSMPL_MED,
        RSMPL_Q1,
        RSMPL_Q3
    };

    /**
         * @brief Get the resampling type from its name
         * @param s string representation of a resampling algorithm
         * @return the corresponding resampling type entry
         */
    static resampling_type from_string(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (s == "near" || s == "nearest") {
            return resampling_type::RSMPL_NEAR;
        } else if (s == "bilinear") {
            return resampling_type::RSMPL_BILINEAR;
        } else if (s == "cubic") {
            return resampling_type::RSMPL_CUBIC;
        } else if (s == "cubicspline") {
            return resampling_type::RSMPL_CUBICSPLINE;
        } else if (s == "lanczos") {
            return resampling_type::RSMPL_LANCZOS;
        } else if (s == "average" || s == "mean") {
            return resampling_type::RSMPL_AVERAGE;
        } else if (s == "mode") {
            return resampling_type::RSMPL_MODE;
        } else if (s == "max") {
            return resampling_type::RSMPL_MAX;
        } else if (s == "min") {
            return resampling_type::RSMPL_MIN;
        } else if (s == "med" || s == "median") {
            return resampling_type::RSMPL_MED;
        } else if (s == "q1") {
            return resampling_type::RSMPL_Q1;
        } else if (s == "q3") {
            return resampling_type::RSMPL_Q3;
        }
        return resampling_type::RSMPL_NEAR;
    }

    /**
         * @brief Get the name of a resampling method
         * @param r resampling type
         * @return name string of the resampling method
         */
    static std::string to_string(resampling_type r) {
        switch (r) {
            case resampling_type::RSMPL_NEAR:
                return "near";
            case resampling_type::RSMPL_BILINEAR:
                return "bilinear";
            case resampling_type::RSMPL_CUBIC:
                return "cubic";
            case resampling_type::RSMPL_CUBICSPLINE:
                return "cubicspline";
            case resampling_type::RSMPL_LANCZOS:
                return "lanczos";
            case resampling_type::RSMPL_AVERAGE:
                return "average";
            case resampling_type::RSMPL_MODE:
                return "mode";
            case resampling_type::RSMPL_MAX:
                return "max";
            case resampling_type::RSMPL_MIN:
                return "min";
            case resampling_type::RSMPL_MED:
                return "med";
            case resampling_type::RSMPL_Q1:
                return "q1";
            case resampling_type::RSMPL_Q3:
                return "q3";
            default:
                return "near";
        }
    }

    /**
         * @brief Convert a resampling type to the corresponding GDAL type to be used in RasterIO
         * @note RasterIO does not support all available resampling types
         * @param r resampling type
         * @return GDALRIOResampleAlg
         */
    static GDALRIOResampleAlg to_gdal_rasterio(resampling_type r) {
        switch (r) {
            case resampling_type::RSMPL_NEAR:
                return GRIORA_NearestNeighbour;
            case resampling_type::RSMPL_BILINEAR:
                return GRIORA_Bilinear;
            case resampling_type::RSMPL_CUBIC:
                return GRIORA_Cubic;
            case resampling_type::RSMPL_CUBICSPLINE:
                return GRIORA_CubicSpline;
            case resampling_type::RSMPL_LANCZOS:
                return GRIORA_Lanczos;
            case resampling_type::RSMPL_AVERAGE:
                return GRIORA_Average;
            case resampling_type::RSMPL_MODE:
                return GRIORA_Mode;
            case resampling_type::RSMPL_MAX:
            case resampling_type::RSMPL_MIN:
            case resampling_type::RSMPL_MED:
            case resampling_type::RSMPL_Q1:
            case resampling_type::RSMPL_Q3:
            default:
                return GRIORA_NearestNeighbour;  // Not yet defined in gdal.h
        }
    }
};

/**
 * @brief Spatial and temporal reference for data cubes
 */
class cube_st_reference {
   public:
    virtual ~cube_st_reference() {
    }

    /**
         * Get the number of cells in x dimension
         * @return number of cells in x dimension
         */
    inline uint32_t &nx() { return _nx; }

    /**
        * Get the number of cells in y dimension
        * @return number of cells in y dimension
        */
    inline uint32_t &ny() { return _ny; }

    /**
        * Get the size of cells in x dimension
        * @return size of cells in x dimension
        */
    inline double dx() { return (_win.right - _win.left) / _nx; }

    /**
        * Set the size of cells in x dimension
        * @note if the width of the spatial window is not a multiple of the new dx, the window will be widened at both ends
        * @param x size of cells in x dimension
        */
    inline void dx(double x) {
        _nx = (uint32_t)std::ceil((_win.right - _win.left) / x);
        double exp_x = _nx * x - (_win.right - _win.left);
        _win.right += exp_x / 2;
        _win.left -= exp_x / 2;
        if (std::fabs(exp_x) > std::numeric_limits<double>::epsilon()) {
            GCBS_INFO("Size of the cube in x direction does not align with dx, extent will be enlarged by " +
                      std::to_string(exp_x / 2) + " at both sides.");
        }
    }

    /**
       * Get the size of cells in y dimension
       * @return size of cells in y dimension
       */
    inline double dy() { return (_win.top - _win.bottom) / _ny; }

    /**
        * Set the size of cells in y dimension
         *@note if the height of the spatial window is not a multiple of the new dy, the window will be widened at both ends
        * @param x size of cells in y dimension
        */
    inline void dy(double x) {
        _ny = (uint32_t)std::ceil((_win.top - _win.bottom) / x);
        double exp_y = _ny * x - (_win.top - _win.bottom);
        _win.top += exp_y / 2;
        _win.bottom -= exp_y / 2;
        if (std::fabs(exp_y) > std::numeric_limits<double>::epsilon()) {
            GCBS_INFO("Size of the cube in y direction does not align with dy, extent will be enlarged by " +
                      std::to_string(exp_y / 2) + " at both sides.");
        }
    }

    /**
         * Get the lower limit in x dimension / left boundary of the cube's extent
         * @return left boundary of the cube's extent
         */
    inline double &left() { return _win.left; }

    /**
         * Get the upper limit in x dimension / right boundary of the cube's extent
         * @return right boundary of the cube's extent
         */
    inline double &right() { return _win.right; }

    /**
         * Get the lower limit in y dimension / bottom boundary of the cube's extent
         * @return bottom boundary of the cube's extent
         */
    inline double &bottom() { return _win.bottom; }

    /**
         * Get the upper limit in y dimension / top boundary of the cube's extent
         * @return top boundary of the cube's extent
         */
    inline double &top() { return _win.top; }

    /**
         * Get or set the spatial reference system / projection
         * @return string (reference) with projection / SRS information that is understandable by GDAL / OGR
         */
    inline std::string &srs() { return _srs; }

    /**
         * Return the spatial reference system / projection
         * @return OGRSpatialReference object
         */
    inline OGRSpatialReference srs_ogr() const {
        OGRSpatialReference s;
        s.SetFromUserInput(_srs.c_str());
        return s;
    }

    /**
         * Getter / setter for the lower boundary of the cube's temporal extent (start datetime)
         * @return reference to the object's t0 object
         */
    inline datetime &t0() { return _t0; }

    /**
         * Getter / setter for the upper boundary of the cube's temporal extent (end datetime)
         * @return reference to the object's t1 object
         */
    inline datetime &t1() { return _t1; }

    /**
         * Get the numbers of cells in the time dimension
         * @return integer number of cells
         */
    uint32_t nt() {
        if (_t1 == _t0) return 1;
        duration d = (_t1 - _t0) + 1;
        return (d % _dt == 0) ? d / _dt : (1 + (d / _dt));
    }

    /**
         * Set the numbers of cells in the time dimension. This method will automatically derive a
         * datetime duration of cells based on the current unit
         */
    void nt(uint32_t n) {
        duration d = (_t1 - _t0) + 1;
        duration dnew = dt();
        if (dnew.dt_interval == 0) {  // if dt has not been set
            dnew.dt_unit = d.dt_unit;
            // alternatively, a "reasonable" should be derived here
        }
        dnew.dt_interval = (int32_t)std::ceil((double)d.dt_interval / (double)n);
        _dt = dnew;
        if (d.dt_interval % n != 0) {
            _t1 = _t0 + _dt * (n - 1);
            GCBS_INFO(
                "Temporal size of the cube does not align with nt, end date/time of the cube will be extended to " +
                _t1.to_string() + ".");
        }
        //
        //        if (nt() == n - 1) {  // in some cases (e.g. d == 9M, n==4), we must extend the temporal extent of the view
        //            _t1 = _t1 + dt();
        //            GCBS_WARN("Extent in t direction is indivisible by nt, end date/time will be set to " + _t1.to_string());
        //        }
        assert(nt() == n);
    }

    /**
     * Get the spatial extent / window of a cube view
     * @return spatial extent, coordinates are expressed in the view's projection / SRS
     */
    inline bounds_2d<double> &win() { return _win; }

    /**
     * Get the temporal size / duration of one cube cell
     * @return the view's dt field as a duration object
     */
    inline duration dt() { return _dt; }

    inline datetime_unit &dt_unit() { return _dt.dt_unit; }

    inline int32_t &dt_interval() { return _dt.dt_interval; }

    /**
    * Set the temporal size / duration of one cube cell
    * @param dt new datetime duration of a cube cell
    */
    void dt(duration dt) {
        //if (dt.dt_unit != _dt.dt_unit) {
        _t0.unit() = dt.dt_unit;
        _t1.unit() = dt.dt_unit;
        //}
        duration dtotal = _t1 - _t0;  // + 1 if include end date2
        dtotal.dt_interval += 1;
        if (dtotal % dt != 0) {
            duration end_duration;  // end duration has one (day / month / unit) less than dt)
            end_duration.dt_interval = dt.dt_interval - 1;
            end_duration.dt_unit = dt.dt_unit;
            _t1 = (_t0 + dt * (dtotal / dt)) + end_duration;
            GCBS_INFO(
                "Temporal size of the cube does not align with dt, end date/time of the cube will be extended to " +
                _t1.to_string());
        }
        _dt = dt;
    }

    /**
     * Set the temporal size of cube cells as n days
     * @param n duration / temporal size of one cell as number of days
     */
    void set_daily(uint16_t n = 1) {
        _dt = duration(n, datetime_unit::DAY);
    }

    /**
     * Set the temporal size of cube cells as n months
     * @param n duration / temporal size of one cell as number of months
     */
    void set_monthly(uint16_t n = 1) {
        _dt = duration(n, datetime_unit::MONTH);
    }

    /**
        * Set the temporal size of cube cells as n years
        * @param n duration / temporal size of one cell as number of years
        */
    void set_yearly(uint16_t n = 1) {
        _dt = duration(n, datetime_unit::YEAR);
    }

    /**
        * Set the temporal size of cube cells as n quarter years
        * @param n duration / temporal size of one cell as number of quarter years
        */
    void set_quarterly(uint16_t n = 1) {
        _dt = duration(3 * n, datetime_unit::MONTH);
    }

    /**
        * Set the temporal size of cube cells as n weeks
        * @param n duration / temporal size of one cell as number of weeks
        */
    void set_weekly(uint16_t n = 1) {
        _dt = duration(n, datetime_unit::WEEK);
    }

    /**
        * Convert integer cube-based coordinates to spacetime coordinates
        * @note cube-based coordinates are in the order (t,y,x), (0,0,0) corresponds to the earliest date (t0) for the
        * lower left pixel.
        * @note Output coordinates will have the projection / SRS as in cube_st_reference::proj()
        * @see cube_st_reference::view_coords()
        * @param p cube-based coordinates
        * @return spacetime coordinates
        */
    coords_st map_coords(coords_nd<uint32_t, 3> p) {
        coords_st s;
        s.s.x = _win.left + p[2] * dx();
        s.s.y = _win.bottom + p[1] * dy();
        s.t = _t0 + _dt * p[0];
        return s;
    }

    /**
         * Convert spacetime coordinates to integer cube-based coordinates
         * @note cube-based coordinates are in the order (t,y,x), (0,0,0) corresponds to the earliest date (t0) for the
         * lower left pixel.
         * @note the function assumes input coordinates have the  projection / SRS as in cube_st_reference::proj()
         * @see cube_st_reference::map_coords()
         * @param p spacetime coordinates
         * @return cube-based coordinates
         */
    coords_nd<uint32_t, 3> cube_coords(coords_st p) {
        coords_nd<uint32_t, 3> s;
        s[2] = (uint32_t)((p.s.x - _win.left) / dx());
        s[1] = (uint32_t)((p.s.y - _win.bottom) / dy());
        s[0] = (uint32_t)((p.t - _t0) / _dt);
        return s;
    }

    friend bool operator==(const cube_st_reference &l, const cube_st_reference &r) {
        if (!(l._win.left == r._win.left &&
              l._win.right == r._win.right &&
              l._win.top == r._win.top &&
              l._win.bottom == r._win.bottom &&
              l._nx == r._nx &&
              l._ny == r._ny &&
              l._t0 == r._t0 &&
              l._dt == r._dt))
            return false;

        // compare SRS
        OGRSpatialReference a = l.srs_ogr();
        OGRSpatialReference b = r.srs_ogr();

        if (!a.IsSame(&b))
            return false;

        return true;
    }

    inline friend bool operator!=(const cube_st_reference &l, const cube_st_reference &r) { return !(l == r); }

   protected:
    /**
         * @brief Spatial reference system / projection
         *
         * The string must be readable for OGRSpatialReference::SetFromUserInput, i.e.,
         * it can be "EPSG:xxx", WKT, or PROJ.4
         *
         */
    std::string _srs;

    /**
         * @brief Spatial window
         */
    bounds_2d<double> _win;

    datetime _t0;
    datetime _t1;
    uint32_t _nx;
    uint32_t _ny;

    duration _dt;
};

/**
 * A data cube view includes the spacetime reference of a cube (extent, resolution, projection) and
 * optional resampling and aggregation algorithms that are applied when original images are
 * read from an image_collection_cube. Aggregation refers to how multiple values for the same
 * cube cell from different images are combined whereas resampling refers to the algorithm used
 * to warp / reproject images to the cube geometry.
 */
class cube_view : public cube_st_reference {
   public:
    /**
         * Deserializes a cube_view object from a JSON file.
         * @param filename Path to the json file on disk
         * @return A cube_view object
         */
    static cube_view read_json(std::string filename);

    /**
        * Deserializes a cube_view object from a JSON file.
        * @param str JSON string
        * @return A cube_view object
        */
    static cube_view read_json_string(std::string str);

    /**
        * Serializes a cube_view object as a JSON file.
        * @param filename output file
        */
    void write_json(std::string filename);

    /**
          * Serializes a cube_view object as a JSON string.
          * @return JSON string
          */
    std::string write_json_string();

    /**
         * Getter / setter for aggregation method
         * @return reference to the object's aggregation field
         */
    inline aggregation::aggregation_type &aggregation_method() { return _aggregation; }

    /**
        * Getter / setter for resampling method
        * @return reference to the object's resampling field
        */
    inline resampling::resampling_type &resampling_method() { return _resampling; }

   private:
    static cube_view read(nlohmann::json j);

    resampling::resampling_type _resampling;
    aggregation::aggregation_type _aggregation;
};

}  // namespace gdalcubes

#endif  //VIEW_H
