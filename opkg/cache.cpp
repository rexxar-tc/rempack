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
#include <unordered_map>

#define  CONTAINS(x,z) ((x).find(z) != (x).end())

namespace fs = filesystem;
using namespace std;

using ByteBuffer = std::vector<uint8_t>;

/**
 *  Extract the first regular file whose path begins with `dirPrefix`.
 *  @param tarGzBuffer  – raw bytes of data.tar.gz already in memory
 *  @param dirPrefix    – e.g. "opt/share/remarkable/splashscreens/"
 *                        (with or without leading "./", with or without trailing '/')
 *  @param outPath      – optional; receives the full path of the file we extracted
 *  @return             – decompressed file data; empty vector if not found / on error
 */
ByteBuffer extractFirstFileByPrefix(const ByteBuffer& tarGzBuffer,
                                    std::string dirPrefix,
                                    std::string* outPath = nullptr)
{
    // Normalise the prefix (strip leading "./" and ensure trailing '/')
    auto stripDotSlash = [](const std::string& s) -> std::string {
        return (s.rfind("./", 0) == 0) ? s.substr(2) : s;
    };
    dirPrefix = stripDotSlash(dirPrefix);
    if (!dirPrefix.empty() && dirPrefix.back() != '/')
        dirPrefix.push_back('/');

    struct archive* a = archive_read_new();
    archive_read_support_filter_gzip(a);
    archive_read_support_format_tar(a);

    if (archive_read_open_memory(a, tarGzBuffer.data(), tarGzBuffer.size()) != ARCHIVE_OK) {
        std::cerr << "archive_read_open_memory: " << archive_error_string(a) << '\n';
        archive_read_free(a);
        return {};
    }

    ByteBuffer result;
    struct archive_entry* entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        std::string path = archive_entry_pathname(entry) ?: "";
        path = stripDotSlash(path);

        // Only care about regular files under the desired prefix
        if (archive_entry_filetype(entry) == AE_IFREG &&
            path.rfind(dirPrefix, 0) == 0)              // prefix match
        {
            size_t size = static_cast<size_t>(archive_entry_size(entry));
            result.resize(size);
            la_ssize_t n = archive_read_data(a, result.data(), size);
            if (n < 0) {
                std::cerr << "archive_read_data: " << archive_error_string(a) << '\n';
                result.clear();
            } else if (outPath) {
                *outPath = path;
            }
            break; // found the first one – stop
        } else {
            archive_read_data_skip(a);
        }
    }

    archive_read_close(a);
    archive_read_free(a);
    return result; // empty if not found
}

std::unordered_map<std::string, ByteBuffer> extractIpkToMemory(const std::string& tarGzPath) {
    std::unordered_map<std::string, ByteBuffer> files;

    struct archive* a = archive_read_new();
    archive_read_support_filter_gzip(a);   // gzip compression
    archive_read_support_format_tar(a);    // tar format

    if (archive_read_open_filename(a, tarGzPath.c_str(), 10240) != ARCHIVE_OK) {
        std::cerr << "Failed to open archive: " << archive_error_string(a) << "\n";
        archive_read_free(a);
        return files;
    }

    struct archive_entry* entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        std::string path = archive_entry_pathname(entry);
        size_t size = archive_entry_size(entry);

        ByteBuffer buffer(size);
        la_ssize_t read = archive_read_data(a, buffer.data(), size);
        if (read < 0) {
            std::cerr << "Error reading file " << path << ": " << archive_error_string(a) << "\n";
        } else {
            //std::cout << "Found file: " << path << " (" << size << " bytes)\n";
            files[path] = std::move(buffer);
        }
    }

    archive_read_close(a);
    archive_read_free(a);

    return files;
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

    auto files = extractIpkToMemory(path);
    auto file = files["./data.tar.gz"];
    return extractFirstFileByPrefix(file, "opt/share/remarkable/splashscreens/");
}

bool opkg::isPackageCached(const shared_ptr<package> &pkg) {
    auto path = get_cached_path() + "/" + pkg->Filename;
    if(!fs::exists(path))
        return false;
    return true;
}
//TODO: need to clean up old versions

