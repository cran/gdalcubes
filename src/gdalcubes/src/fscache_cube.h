/*
   Copyright 2024 Marius Appel <marius.appel@hs-bochum.de>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef FSCACHE_H
#define FSCACHE_H

#include <algorithm>
#include <string>

#include <sqlite3.h>

#include "cube.h"


namespace gdalcubes {

/**
     * @brief A data cube chunk cache implementation on the local file system, e.g. on tmpfs
     *
     * IDEA:
     * - Use an SQL database that is shared among all workers with a table (chunk_id, path_to_file, last_access, ...)
     */
class fscache_cube : public cube {
   public:


    static std::shared_ptr<fscache_cube> create(std::shared_ptr<cube> in, std::string path, uint64_t max_size_bytes) {
        std::shared_ptr<fscache_cube> out = std::make_shared<fscache_cube>(in, predicate);
        in->add_child_cube(out);
        out->add_parent_cube(in);
        return out;
    }

   public:

    fscache_cube(std::shared_ptr<cube> in, std::string path, uint64_t max_size_bytes) : cube(in->st_reference()->copy()), _in_cube(in), 
                                                                    _path(path), _max_size_bytes(max_size_bytes) {  // it is important to duplicate st reference here, otherwise changes will affect input cube as well
        _chunk_size[0] = _in_cube->chunk_size()[0];
        _chunk_size[1] = _in_cube->chunk_size()[1];
        _chunk_size[2] = _in_cube->chunk_size()[2];


        _wid = utils::env::get("WORKER_ID");
        if (_wid.empty()) {
            _wid = utils::generate_unique_filename(8); // random string
        }

        // Create dir?
        // TODO: if WORKER_ID == 0

        int sqlite3_open(
            const char *filename,   /* Database filename (UTF-8) */
            sqlite3 **ppDb          /* OUT: SQLite db handle */
        );


    }

   public:
    ~fscache_cube() {}

    std::shared_ptr<chunk_data> read_chunk(chunkid_t id) override;

    json11::Json make_constructible_json() override {
        json11::Json::object out;
        out["cube_type"] = "fscache";
        out["path"] = _path;
        out["max_size_bytes"] = _max_size_bytes;
        out["in_cube"] = _in_cube->make_constructible_json();
        return out;
    }

   private:
    std::shared_ptr<cube> _in_cube;
    std::string _path;
    uint64_t _max_size_bytes;
    std::string _wid;


};

}  // namespace gdalcubes

#endif  // FSCACHE_H
