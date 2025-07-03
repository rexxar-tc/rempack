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

static string get_cached_path(const string& basename = "rempack"){
    const char* xdg_cache_home = std::getenv("XDG_CACHE_HOME");
    string base = xdg_cache_home ? xdg_cache_home : std::getenv("HOME") + std::string("/.cache");
    string path =  base + "/" + basename;
    if(!fs::exists(path))
        fs::create_directories(path);
    return path;
}

string opkg::getCachedSplashscreen(const shared_ptr<package>& pkg) {
    auto path = get_cached_path() + "/" + pkg->Filename;
    if(fs::exists(path))
        return path;

    return DownloadPackage(pkg, nullptr, get_cached_path());
}

//TODO: need to clean up old versions