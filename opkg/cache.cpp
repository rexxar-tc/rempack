//
// Created by brant on 7/1/25.
//

#include "opkg.h"
#include <filesystem>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <archive.h>
#include <archive_entry.h>

#define  CONTAINS(x,z) ((x).find(z) != (x).end())

namespace fs = filesystem;
using namespace std;

vector<uint8_t> extractImage(const std::string& ipkPath, std::string dirPrefix) {
    // Normalise the prefix (strip leading "./" and ensure trailing '/')
    auto stripDotSlash = [](const std::string& s) -> std::string {
        return (s.rfind("./", 0) == 0) ? s.substr(2) : s;
    };
    dirPrefix = stripDotSlash(dirPrefix);
    if (!dirPrefix.empty() && dirPrefix.back() != '/')
        dirPrefix.push_back('/');

    struct archive* a = archive_read_new();
    archive_read_support_filter_gzip(a);   // gzip compression
    archive_read_support_format_tar(a);    // tar format

    if (archive_read_open_filename(a, ipkPath.c_str(), 10240) != ARCHIVE_OK) {
        std::cerr << "Failed to open archive: " << archive_error_string(a) << "\n";
        archive_read_free(a);
        return {};
    }

    archive_entry* entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        std::string path = archive_entry_pathname(entry);
        if(path.find("data.tar.gz") == string::npos) {
            archive_read_data_skip(a);
            continue;
        }
        size_t size = archive_entry_size(entry);

        vector<uint8_t> buffer(size);
        la_ssize_t read = archive_read_data(a, buffer.data(), size);
        if (read < 0) {
            std::cerr << "Error reading file " << path << ": " << archive_error_string(a) << "\n";
        } else {

            struct archive* ab = archive_read_new();
            archive_read_support_filter_gzip(ab);
            archive_read_support_format_tar(ab);

            if (archive_read_open_memory(ab, buffer.data(), buffer.size()) != ARCHIVE_OK) {
                std::cerr << "archive_read_open_memory: " << archive_error_string(ab) << '\n';
                archive_read_close(ab);
                archive_read_free(ab);
                return {};
            }

            vector<uint8_t> result;
            struct archive_entry* innerEntry;
            while (archive_read_next_header(ab, &innerEntry) == ARCHIVE_OK) {
                std::string innerPath = archive_entry_pathname(innerEntry) ?: "";
                innerPath = stripDotSlash(innerPath);

                // Only care about regular files under the desired prefix
                if (archive_entry_filetype(innerEntry) == AE_IFREG &&
                    innerPath.rfind(dirPrefix, 0) == 0)              // prefix match
                {
                    auto innerSize = static_cast<size_t>(archive_entry_size(innerEntry));
                    result.resize(innerSize);
                    la_ssize_t n = archive_read_data(ab, result.data(), innerSize);
                    if (n < 0) {
                        std::cerr << "archive_read_data: " << archive_error_string(ab) << '\n';
                        result.clear();
                    }
                    break; // found the first one – stop
                } else {
                    archive_read_data_skip(ab);
                }
            }

            archive_read_close(ab);
            archive_read_free(ab);
            archive_read_close(a);
            archive_read_free(a);
            return result; // empty if not found
        }
    }

    archive_read_close(a);
    archive_read_free(a);

    return {};
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
    if(!fs::exists(path))
      path = DownloadPackage(pkg, nullptr, get_cached_path());

    return extractImage(path, "opt/share/remarkable/splashscreens/");
}

bool opkg::isPackageCached(const shared_ptr<package> &pkg) {
    auto path = get_cached_path() + "/" + pkg->Filename;
    if(!fs::exists(path))
        return false;
    return true;
}
//TODO: need to clean up old versions

