//
// Created by brant on 2/2/24.
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
#include <queue>


#define  CONTAINS(x,z) ((x).find(z) != (x).end())

namespace fs = filesystem;
using namespace std;

/*
 * Package: toltec-base
 * Description: Metapackage defining the base set of packages in a Toltec install
 * Homepage: https://toltec-dev.org/
 * Version: 1.2-2
 * Section: utils
 * Maintainer: Eeems <eeems@eeems.email>
 * License: MIT
 * Architecture: rm2
 * Depends: toltec-completion, toltec-bootstrap, rm2-suspend-fix
 * Filename: toltec-base_1.2-2_rm2.ipk
 * SHA256sum: f5799454493c88b3018732ec16b5585a81ad8ec63deb1806b77ca34eea2080e8
 * Size: 2122
 */

const fs::path OPKG_DB{"/opt/var/opkg-lists"};
const fs::path OPKG_LIB{"/opt/lib/opkg"}; //info(dir) lists(dir(empty?)) status(f)

#include <mutex>
#include <condition_variable>
#include <thread>

class SimpleLatch {
public:
    explicit SimpleLatch(int count) : count_(count) {}

    void count_down() {
        std::unique_lock<std::mutex> lock(mutex_);
        if (--count_ == 0) {
            cv_.notify_all();
        }
    }

    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&] { return count_ == 0; });
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    int count_;
};

//need to remove LD_PRELOAD var set by rm2fb-client:
std::unordered_set<std::string> preload_excludes = {"/opt/lib/librm2fb_client.so", "/opt/lib/librm2fb_client.so.1", "/opt/lib/libsysfs_preload.so"};
int execute(const std::string& cmd, const function<void (const std::string &)> &callback) {
    std::string preloadStr_original = getenv("LD_PRELOAD");

    //std::cerr << "PRELOAD: " << preloadStr_original << std::endl;

    std::stringstream preloadSs;
    auto lds = utils::split_str(preloadStr_original, ':');
    for(const auto &ld : lds){
        if(CONTAINS(preload_excludes, ld))
            continue;
        preloadSs << ld << ':';
    }
    auto preloadstr = preloadSs.str();
    preloadstr = preloadstr.substr(0,preloadstr.size()-1);
    //setenv("LD_PRELOAD", preloadstr.c_str(), 1);
    std::stringstream invocation;
    invocation << "source ~/.bashrc ; ";
    invocation << cmd << " 2>&1";
    auto pipe = popen(invocation.str().c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed!");

    char cline[4096];
    while (fgets(cline, sizeof(cline), pipe)){
        if(callback) {
            for (const auto c: cline) {
                if (c == 0)
                    break;
                if (c == '\n') {
                    auto s = std::string(cline);
                    if (s.find("LD_PRELOAD") == std::string::npos) //just squelch all LD_PRELOAD errors
                        callback(s);
                    else
                        std::cerr << s << std::endl;
                    memset(cline, 0, sizeof(cline));
                    break;
                }
            }
        }
    }

    //setenv("LD_PRELOAD", preloadStr_original.c_str(), 1);

    return pclose(pipe);
}

int opkg::Install(const vector<shared_ptr<package>> &targets, const function<void(const string &)> &lineCallback, const std::string& args) {
    stringstream ss;
    ss << "opkg install ";
    for(const auto& p: targets)
        ss << p->Package << " ";
    if(OPKG_FORCE_NOACTION)
        ss << " --noaction";
    ss << args;
    auto ret = execute(ss.str(), lineCallback);

    //TODO: this isn't quite right, we need to rescan control files
    //mark targets as installed for UI niceness
    if(ret == 0){
        for(const auto &t : targets)
            t->State = package::Installed;
    }
    return ret;
}

int opkg::Uninstall(const vector<shared_ptr<package>> &targets, const function<void(const string &)> &lineCallback, const std::string& args) {
    stringstream ss;
    ss << "opkg remove ";
    for(const auto& p: targets)
        ss << p->Package << " ";
    if(OPKG_FORCE_NOACTION)
        ss << " --noaction";
    ss << args;
    auto ret = execute(ss.str(), lineCallback);
    //if(ret == 0){
    //    for(const auto &t : targets)
    //        t->State = package::NotInstalled;
    //}
    return ret;
}

int opkg::UpdateRepos(const function<void(const string &)> &lineCallback) {
    return execute("opkg update", lineCallback);
}

string opkg::DownloadPackage(const shared_ptr<package> &pkg, const function<void(const string &)> &lineCallback, const string &targetPath) {
    execute("cd " + targetPath + " ; opkg download " + pkg->Package, lineCallback);
    return targetPath + "/" + pkg->Filename;
}

void opkg::update_lists(){
    unordered_set<string> _sections;
    for (const auto& [n, pk]: packages){
        _sections.emplace(pk->Section);
        sections_by_repo[pk->Section].emplace(pk->Repo);
    }
    for(const auto &s: _sections)
        sections.push_back(s);
}

void opkg::LoadSections(vector<string> *categories, vector<string> excludeRepos) {
    unordered_set<string> set;
    for (const auto& [n, pk]: packages) {
        if(!excludeRepos.empty() && find(excludeRepos.begin(), excludeRepos.end(), pk->Repo) == excludeRepos.end())
            continue;
        if(!pk->Section.empty())
            set.emplace(pk->Section);
    }
    for (const auto &section: set) {
        categories->push_back(section);
    }
}

void opkg::LoadPackages(vector<string> *dest, vector<string> excludeRepos) {
    for (const auto& [n, pk]: packages) {
        if(!excludeRepos.empty() && find(excludeRepos.begin(), excludeRepos.end(), pk->Repo) == excludeRepos.end())
            continue;
        dest->emplace_back(pk->Package);
    }
}

bool opkg::split_str_and_find(const string& children_str, vector<shared_ptr<package>> &field){
    auto splits = utils::split_str(children_str, ',');
    if(splits.empty()){
        return false;
    }
    bool err = false;
    for(const auto &s : splits){
        auto it = packages.find(s);
        if(it == packages.cend())
        {
            //dependency may have a version in the string: Failed to resolve dependency tarnish (= 2.6-3) for package oxide-utils
            //strip the version here and try again. This seems to catch everything
            auto dsplit = utils::split_str(s, ' ');
            if(!dsplit.empty()){
                it = packages.find(dsplit[0]);
                if(it != packages.cend()){
                    field.push_back(it->second);
                    continue;
                }

                auto vpk = make_shared<package>(package{
                        .Package = dsplit[0],
                        .Virtual = true
                });

                packages.emplace(dsplit[0], vpk);
                field.push_back(vpk);
                //printf("Creating virtual package %s\n", dsplit[0].c_str());
                continue;
            }
            err = true;
            //printf("Failed to resolve child %s for package\n", s.c_str());
            continue;
        }
        field.push_back(it->second);
    }
    return !err;
}

//after all packages are parsed, just roll through the list once to link dependent packages
void opkg::link_dependencies(){
    for(const auto &[n, pkg] : packages){
        if(!pkg->_depends_str.empty()) {
            if (!split_str_and_find(pkg->_depends_str, pkg->Depends)) {
                //printf("Problem resolving dependencies for package %s\n", pkg->Package.c_str());
            }
            for(const auto &dpk: pkg->Depends){
                dpk->Dependents.push_back(pkg);
            }
        }
        if(!pkg->_recommends_str.empty()){
            if (!split_str_and_find(pkg->_recommends_str, pkg->Recommends)) {
                //printf("Problem resolving recommendations for package %s\n", pkg->Package.c_str());
            }
        }
        if(!pkg->_conflicts_str.empty()){
            if (!split_str_and_find(pkg->_conflicts_str, pkg->Conflicts)) {
                //printf("Problem resolving conflicts for package %s\n", pkg->Package.c_str());
            }
        }
        if(!pkg->_replaces_str.empty()){
            if (!split_str_and_find(pkg->_replaces_str, pkg->Replaces)) {
                //printf("Problem resolving replacing for package %s\n", pkg->Package.c_str());
            }
        }
        if(!pkg->_provides_str.empty()){
            if (!split_str_and_find(pkg->_provides_str, pkg->Provides)) {
                //printf("Problem resolving provides for package %s\n", pkg->Package.c_str());
            }
        }
    }
}

void opkg::update_states() {
    for(auto const &[name,pkg] : packages){
        if(pkg->_status_str.empty()){
            pkg->State = package::NotInstalled;
            continue;
        }
        auto status = pkg->_status_str.c_str();

        char sw_str[64], sf_str[64], ss_str[64];
        int r;

        r = sscanf(status, "%63s %63s %63s", sw_str, sf_str, ss_str);
        if(r != 3){
            printf("Error parsing state for %s : %s\n", pkg->Package.c_str(), status);
            continue;
        }

        if(strcmp(sw_str, "install") == 0){
            if(strcmp(ss_str, "installed")== 0){
                pkg->State = package::Installed;
                continue;
            }
        }

        pkg->State = package::InstallError;
    }
}

static void scanRepos(const filesystem::directory_entry &f, map<string, shared_ptr<package>> &packages){
    printf("extracting archive %s\n", f.path().c_str());
    auto gzf = gzopen(f.path().c_str(), "rb");
    int count = 0;
    int pc = 0;
    char cbuf[4096]{};
    auto pk = make_shared<package>();
    bool parsing_desc = false;
    bool parsing_conf = false;
    string lastLine;
    //gzgets reads one line out of a gzipped file
    while (gzgets(gzf, cbuf, sizeof(cbuf)) != nullptr) {
        count++;
        if (!opkg::parse_line(pk, packages, cbuf, false, true, parsing_desc, parsing_conf, lastLine)) {     //if parse_line returns false, we're done parsing this package
            parsing_desc = false;
            parsing_conf = false;
            if (pk->Package.empty())
                continue;

            pk->Repo = f.path().filename();
            pc++;
            auto mp = packages.emplace(pk->Package, pk);
            if (!mp.second) {
                printf("emplacement failed for package %d: %s\n", pc, pk->Package.c_str());
            }
            pk = make_shared<package>();
            continue;
        }
    }
    printf("Read %d lines\n", count);
}

static void scanControl(const filesystem::directory_entry &f, map<string, shared_ptr<package>> &packages, int &pc){
    bool parsing_desc = false;
    bool parsing_conf = false;
    ifstream cfile;
    cfile.open(f.path(), ios::in);
    if (!cfile.is_open()) {
        printf("ERROR! Failed to open control file %s\n", f.path().c_str());
        return;
    }
    auto pname = f.path().filename().string().substr(0, f.path().filename().string().find_last_of('.'));
    auto pit = packages.find(pname);
    if (pit == packages.end()) {
        printf("ERROR! Could not match package %s to control file %s\n", pname.c_str(), f.path().c_str());
        return;
    }
    auto pk = pit->second;
    pk->State = package::Installed;
    string lastLine;
    for (string line; getline(cfile, line);) {
        pc++;
        if(!opkg::parse_line(pk, packages, line.c_str(), false, false, parsing_desc, parsing_conf, lastLine)) {    //no need to update extant, we know what package this is from the filename
            parsing_desc = false;
            parsing_conf = false;
        }
    }
}


void opkg::InitializeRepositories(const std::function<void()> &callback) {
    try {
        auto th = new thread([=]() {
            {
                init_repos_internal();
                callback();
            }
        });
        th->detach();
    }
    catch (const std::exception &e) {
        std::cerr << "OPKG THREAD EXC" << ' ' << e.what() << std::endl;
    }
}

void opkg::init_repos_internal()
{
//TODO: This can most likely be parallelized to shorten the startup delay on multicore devices
    packages.clear();
    repositories.clear();
    packages_by_repo.clear();
    int pc = 0;
    auto pk = make_shared<package>();

    auto rn = std::count_if(fs::directory_iterator(OPKG_DB),
                  fs::directory_iterator{},
                  [](const fs::directory_entry& entry) {
                      return entry.is_regular_file();
                  });
    auto repo_latch = make_shared<SimpleLatch>(rn);
    for (const auto &f: fs::directory_iterator(OPKG_DB)) {
        auto md = packages_by_repo.emplace(f.path().filename(), map<string, shared_ptr<package>>{});
        std::cout << "repo" << std::endl;
        (new thread([=]() {
            std::cout << "repo scan" << std::endl;
            scanRepos(f, md.first->second);
            repo_latch->count_down();
            std::cout << "latched" <<std::endl;
        }))->detach();
    }
    //blocking this thread here also blocks all task queue items
    //we need to just launch a thread
    repo_latch->wait();
    for(const auto &r : packages_by_repo){
        repositories.push_back(r.first);
        for(const auto &rp : r.second){
            auto mp = packages.emplace(rp);
            if(!mp.second)
                printf("emplacement failed for package %d: %s\n", pc, rp.second->Package.c_str());
            pc++;
        }
    }
    printf("Parsed %d packages\n", pc);

    pc = 0;
    auto infopath = OPKG_LIB;
    infopath += "/info";
    int fc = 0;

    for (const auto &f: fs::directory_iterator(infopath)) {
        if (f.path().extension() == ".control") {
            fc++;
            scanControl(f, packages, pc);
        }
    }
    printf("Processed %d control files containing %d lines\n", fc, pc);
    printf("Processed %d packages\n", packages.size());

    link_dependencies();
    update_lists();
    update_states();

}

std::unordered_set<std::string> uninstall_cache;
void uninstall_callback(const std::string& line) {
    size_t prefixPos = line.find("Removing package ");
    if (prefixPos != string::npos) {
        // Extract substring starting after "Removing package "
        string packagePart = line.substr(prefixPos + 17);
        // Find the first space to get the package name
        size_t spacePos = packagePart.find(' ');
        if (spacePos != string::npos) {
            uninstall_cache.insert(packagePart.substr(0, spacePos));
        }
    }
}

int opkg::ComputeUninstall(const vector<shared_ptr<package>>& targets, bool includeDependencies, vector<shared_ptr<package>> *out_result) {
    uninstall_cache.clear();

    string s = " --noaction";
    if(includeDependencies)
        s += " --autoremove";

    auto ret = Uninstall(targets, uninstall_callback, s);
    for(const auto& pk : uninstall_cache){
        auto it = packages.find(pk);
        if(it == packages.cend()){
            printf("ERROR! No package found for %s\n", pk.c_str());
            continue;
        }
        out_result->push_back(it->second);
    }
    uninstall_cache.clear();
    return ret;
}
