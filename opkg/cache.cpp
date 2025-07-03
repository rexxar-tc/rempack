//
// Created by brant on 7/1/25.
//

#include "opkg.h"
#include <cstdio>
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstring>
#include "zlib.h"
#include "utils.h"
#include <sstream>
#include <archive.h>

#define  CONTAINS(x,z) ((x).find(z) != (x).end())

namespace fs = filesystem;
using namespace std;

#include <archive.h>
#include <archive_entry.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>

static std::vector<uint8_t> extract_file_from_ipk(const std::string& ipk_path, const std::string& known_path = "opt/share/remarkable/splashscreens/") {
    std::vector<uint8_t> result;

    // Step 1: Extract data.tar.gz from the ipk
    struct archive* ipk = archive_read_new();
    archive_read_support_format_ar(ipk);
    archive_read_support_filter_all(ipk);

    if (archive_read_open_filename(ipk, ipk_path.c_str(), 10240) != ARCHIVE_OK) {
        throw std::runtime_error("Failed to open IPK file");
    }

    struct archive_entry* entry;
    std::stringstream data_tar_gz_stream;

    while (archive_read_next_header(ipk, &entry) == ARCHIVE_OK) {
        std::string name = archive_entry_pathname(entry);
        if (name.find("data") == 0) { // matches "data.tar.gz" or "data.gz"
            const void* buff;
            size_t size;
            int64_t offset;
            while (archive_read_data_block(ipk, &buff, &size, &offset) == ARCHIVE_OK) {
                data_tar_gz_stream.write(reinterpret_cast<const char*>(buff), size);
            }
            break;
        }
        archive_read_data_skip(ipk);
    }
    archive_read_free(ipk);

    // Step 2: Read the data.tar.gz from memory
    std::string data_tar_gz = data_tar_gz_stream.str();
    struct archive* data = archive_read_new();
    archive_read_support_format_tar(data);
    archive_read_support_filter_gzip(data);

    if (archive_read_open_memory(data, data_tar_gz.data(), data_tar_gz.size()) != ARCHIVE_OK) {
        throw std::runtime_error("Failed to open data archive");
    }

    // Step 3: Find the file with the matching path prefix
    while (archive_read_next_header(data, &entry) == ARCHIVE_OK) {
        std::string path = archive_entry_pathname(entry);
        if (path == known_path || path.find(known_path + "/") == 0) {
            size_t size = archive_entry_size(entry);
            result.resize(size);
            archive_read_data(data, result.data(), size);
            break;
        } else {
            archive_read_data_skip(data);
        }
    }

    archive_read_free(data);
    return result;
}

static string get_cached_path(const string& basename = "rempack"){
    const char* xdg_cache_home = std::getenv("XDG_CACHE_HOME");
    string base = xdg_cache_home ? xdg_cache_home : std::getenv("HOME") + std::string("/.cache");
    string path =  base + "/" + basename;
    if(!fs::exists(path))
        fs::create_directories(path);
    return path;
}

vector<uint8_t> opkg::getCachedSplashscreen(const shared_ptr<package>& pkg) {
    auto path = get_cached_path() + "/" + pkg->Filename;
    if(fs::exists(path))
        return extract_file_from_ipk(path);

    return extract_file_from_ipk(DownloadPackage(pkg, nullptr, get_cached_path()));
}

//TODO: need to clean up old versions

