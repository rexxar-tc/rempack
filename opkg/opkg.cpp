//
// Created by brant on 2/2/24.
//

#include "opkg.h"
#include <stdio.h>
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <string>
#include <map>
#include <cstring>
#include "zlib.h"
#include "utils.h"
#include <signal.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stack>

#define  CONTAINS(x,z) (x.find(z) != x.end())

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
unordered_set<string> formatExcludeNames{"libc", "libgcc"};

//need to remove LD_PRELOAD var set by rm2fb-client:
//LD_PRELOAD=/opt/lib/librm2fb_client.so.1
int execute(const std::string& cmd, std::string& output) {
    const string preload = "/opt/lib/librm2fb_client.so.1";
    const int bufsize=128;
    std::array<char, bufsize> buffer{};

    stringstream ss;
    ss << "source ~/.bashrc; ";
#if DEBUG
    ss << "export LD_PRELOAD=${LD_PRELOAD%" << preload << "}; ";
#endif
    ss << cmd << " 2>&1 ; ";
#if DEBUG
    ss << "export LD_PRELOAD=${LD_PRELOAD}:" << preload;
#endif

    auto pipe = popen(ss.str().c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed!");

    size_t count;
    do {
        if ((count = fread(buffer.data(), 1, bufsize, pipe)) > 0) {
            output.insert(output.end(), std::begin(buffer), std::next(std::begin(buffer), count));
        }
    } while(count > 0);

    return pclose(pipe);
};

int execute(const std::string& cmd, const function<void (const std::string &)> &callback) {
    const std::string preload = "/opt/lib/librm2fb_client.so.1";
    std::stringstream ss;
    ss << "source ~/.bashrc; ";
#if DEBUG
    ss << "export LD_PRELOAD=${LD_PRELOAD%" << preload << "}; ";
#endif
    ss << cmd << " 2>&1; ";
#if DEBUG
    ss << "export LD_PRELOAD=${LD_PRELOAD}:" << preload;
#endif
    auto pipe = popen(ss.str().c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed!");

    char cline[4096];
    while (fgets(cline, sizeof(cline), pipe)){
        for(const auto c : cline){
            if(c == 0)
                break;
            if(c == '\n'){
                auto s = std::string(cline);
                callback(s);
                memset(cline, 0, sizeof(cline));
                break;
            }
        }
    }

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
    return execute(ss.str(), lineCallback);
}

int opkg::Uninstall(const vector<shared_ptr<package>> &targets, const function<void(const string &)> &lineCallback, const std::string& args) {
    stringstream ss;
    ss << "opkg remove ";
    for(const auto& p: targets)
        ss << p->Package << " ";
    if(OPKG_FORCE_NOACTION)
        ss << " --noaction";
    ss << args;
    return execute(ss.str(), lineCallback);
}

int opkg::UpdateRepos(const function<void(const string &)> &lineCallback) {
    return execute("opkg update", lineCallback);
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


string opkg::FormatPackage(const shared_ptr<package> &pk) {
    stringstream ss;

    if(!pk->Package.empty()) ss << "Package" << ": " << pk->Package << endl;
    if(!pk->Description.empty()) ss << "Description" << ": " << pk->Description << endl;
    if(!pk->Homepage.empty()) ss << "Homepage" << ": " << pk->Homepage << endl;
    if(!pk->InstalledVersion.empty()) ss << "Installed Version" << ": " << pk->InstalledVersion << endl;
    if(!pk->UpstreamVersion.empty()) ss << "Available Version" << ": " << pk->UpstreamVersion << endl;
    if(!pk->Maintainer.empty()) ss << "Maintainer" << ": " << pk->Maintainer << endl;
    if(!pk->Architecture.empty()) ss << "Architecture" << ": " << pk->Architecture << endl;
    if(!pk->Repo.empty()) ss << "Repo" << ": " << pk->Repo << endl;

    if(pk->IsInstalled()) {
        ss << "Status: Installed" << endl;
        time_t intime = pk->installTime;
        auto tm = localtime(&intime);
        ss << "Installed on " << asctime(tm) << endl;

        if (pk->autoInstalled)
            ss << "AutoInstalled: yes" << endl;

        if(pk->Essential)
            ss << "Essential: yes" << endl;

        ss << "Installed size: " << pk->Size << endl;
    }
    else if(pk->State == package::InstallError){
        ss << "Status: Installation error! Placeholder text!" << endl;
    }
    else{
        ss << "Status: Not installed" << endl;
    }

    if(!pk->Depends.empty()){
        ss << "Depends: ";
        for(const auto &d: pk->Depends)
            ss << d->Package << " ";
        ss << endl;
    }
    if(!pk->Dependents.empty()){
        ss << "Depended by: ";
        for(const auto &d: pk->Dependents)
            if(d->IsInstalled())
                ss << d->Package << " ";
        ss << endl;
    }
    if(!pk->Conflicts.empty()){
        ss << "Conflicts with: ";
        for(const auto &c : pk->Conflicts)
            if(c->IsInstalled())
                ss << c->Package << " ";
        ss << endl;
    }
    if(!pk->Provides.empty()){
        ss << "Provides: ";
        for(const auto &p : pk->Provides)
            ss << p->Package << " ";
        ss << endl;
    }
    return ss.str();
}

bool existsInChildren(const shared_ptr<package> &pkg, vector<shared_ptr<package>> &set, bool top, unordered_set<string> &visited){
    if(set.empty())
        return false;
    for(const auto &dpkg: set){
        if(!visited.emplace(dpkg->Package).second)
            continue;
        if(CONTAINS(formatExcludeNames, dpkg->Package))
            continue;
        if(dpkg == pkg) {
            if (!top)
                return true;
            continue;
        }
        if(dpkg->Depends.empty())
            continue;
        if(existsInChildren(pkg, dpkg->Depends, false, visited))
            return true;
    }
    return false;
}

inline bool hasNextLine(vector<shared_ptr<package>> &set, int offset, bool excludeInstalled, unordered_set<string> &visited){
    if(offset >= set.size())
        return false;
    for(auto it = set.begin() + offset; it < set.end(); it++){
        auto dpkg = *it;
       //if(CONTAINS(visited, dpkg->Package))
       //    return true;
        if(CONTAINS(formatExcludeNames, dpkg->Package))
            continue;
        if(excludeInstalled && dpkg->IsInstalled())
            continue;
        unordered_set<string> cps;
        if(existsInChildren(dpkg, set, true, cps))
            continue;
        return true;
    }
    return false;
}

static void recurseDependencyTree(const shared_ptr<package>& pkg, stringstream &ss, bool excludeInstalled, stringstream &prefix, unordered_set<string> &visited){
    if(excludeInstalled && pkg->IsInstalled())
        return;
    if(CONTAINS(formatExcludeNames, pkg->Package))
        return;

    ss << prefix.str() << '-' << pkg->Package;// << endl;
    if(pkg->Depends.empty()) {
        ss << endl;
        return;
    }
    ss << " (";
    for(const auto &dpk: pkg->Depends)
        ss << dpk->Package << ", ";
    ss.seekp(-2, std::ios_base::end);
    ss << ")" << endl;

    prefix << "|";
    for(int i = 0; i < pkg->Depends.size(); i++) {
        if (!hasNextLine(pkg->Depends, i, excludeInstalled, visited)) {
            prefix.seekp(-2, std::ios_base::end);
            prefix << ".:";
        }
        auto &dpkg = pkg->Depends.at(i);
        if (!visited.emplace(dpkg->Package).second) {
            ss << prefix.str() << '-' << dpkg->Package << endl;
        } else
            recurseDependencyTree(dpkg, ss, excludeInstalled, prefix, visited);
    }
    auto lpx = prefix.str().substr(0, std::max((size_t)1, prefix.str().length() - 1));
    prefix.str(lpx);
}

string opkg::formatDependencyTree(const shared_ptr<package>& pkg, bool excludeInstalled) {
    if (pkg->Depends.empty())
        return "";
    stringstream ss;
    stringstream prefix;
    unordered_set<string> visited;
    recurseDependencyTree(pkg, ss, excludeInstalled, prefix, visited);
    return ss.str();
}

//TODO: this needs to move to utilities
vector<string> split_str(const string &s, const char delimiter)
{
    vector<string> splits;
    string _split;
    istringstream ss(s);
    while (getline(ss, _split, delimiter))
    {
        utils::ltrim(_split);
        splits.push_back(_split);
    }
    return splits;
}

bool opkg::split_str_and_find(const string& children_str, vector<shared_ptr<package>> &field){
    auto splits = split_str(children_str, ',');
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
            auto dsplit = split_str(s, ' ');
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

inline bool try_parse_str(const char* prefix, const char *line, string &field){
    if (strncmp(line, prefix, strlen(prefix)) != 0)
        return false;
    auto f = string(line + strlen(prefix) + 1);
    utils::trim(f);
    field = f;
    return true;
}

inline bool try_parse_bool(const char *prefix, const char* line, bool &field){
   string s;
    if(try_parse_str(prefix, line, s)){
        if(strncmp(s.c_str(),"yes",3) == 0) {
            field = true;
            return true;
        }
    }
    return false;
}

inline bool try_parse_uint(const char *prefix, const char *line, uint &field) {
    string f;
    if (!try_parse_str(prefix, line, f))
        return false;
    try {
        field = stoul(f);
        return true;
    }
    catch (exception&) {
        return false;
    }
}

inline bool try_parse_long(const char *prefix, const char *line, long &field) {
    string f;
    if (!try_parse_str(prefix, line, f))
        return false;
    try {
        field = stol(f);
        return true;
    }
    catch (exception&) {
        return false;
    }
}

string lastLine;

//this is mostly copied from opkg's own parser. I don't hate it?
bool opkg::parse_line(shared_ptr<package> &ptr, const char *line, bool update, bool upstream) {
    static bool parsing_desc = false; //this is ugly, but it's what opkg does so whatever I guess
    static bool parsing_conf = false; //this is ugly, but it's what opkg does so whatever I guess
    if (ptr == nullptr)
        return false;
    switch (*line) {
        case 'P': {
            // PACKAGE NAME HANDLING
            if (!update) {
                if (try_parse_str("Package", line, ptr->Package)) {     //we're parsing packages out of multiple disparate lists
                    parsing_desc = false;                               //check if we already have a matching package
                    parsing_conf = false;                               //if we do, reset the package pointer to the extant package
                    break;                                              //then we can just keep processing as normal
                }                                                       //this works because Package is always the first line
            }                                                           //in the entry
            else {
                string pn;
                if (try_parse_str("Package", line, pn)) {
                    auto it = packages.find(pn);
                    if (it != packages.end()) {
                        auto &fpk = it->second;
                        ptr = fpk;
                        parsing_desc = false;
                        parsing_conf = false;
                        break;
                    } else {
                        //TODO: actually handle this error?
                        printf("ERROR! PACKAGE NOT FOUND! %s\n", pn.c_str());
                        break;
                    }
                }
            }
            // /PACKAGE NAME HANDLING
            if(try_parse_str("Provides", line, ptr->_provides_str)){
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'A': {
            if (try_parse_str("Architecture", line, ptr->Architecture)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_str("Alternatives", line, ptr->Alternatives)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_bool("Auto-Installed", line, ptr->autoInstalled)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'D': {
            if (try_parse_str("Description", line, ptr->Description)) {
                parsing_desc = true;
                parsing_conf = false;
                break;
            }
            if (try_parse_str("Depends", line, ptr->_depends_str)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'C': {
            if(try_parse_str("Conflicts", line, ptr->_conflicts_str)){
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            string c;
            if (try_parse_str("CPE-ID", line, c)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_str("Conffiles", line, c)) {  //we don't need conffiles, only one package has it
                parsing_desc = false;                   //this will create some errors on the following lines
                                                        //but that's manageable
                parsing_conf = true;                    //the above was a lie, let's handle this anyway
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'E': {
            if (try_parse_bool("Essential", line, ptr->Essential)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'F': {
            if (try_parse_str("Filename", line, ptr->Filename)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'H': {
            if (try_parse_str("Homepage", line, ptr->Homepage)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'I': {
            if (try_parse_uint("Installed-Size", line, ptr->Size)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_long("Installed-Time", line, ptr->installTime)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'L': {
            if (try_parse_str("License", line, ptr->License)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'M': {
            if (try_parse_str("Maintainer", line, ptr->Maintainer)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'R': {
            if (try_parse_str("Replaces", line, ptr->_replaces_str)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            string r;
            if (try_parse_str("Require-User", line, r)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'S': {
            if (try_parse_str("Section", line, ptr->Section)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_str("SHA256sum", line, ptr->SHA256sum)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_uint("Size", line, ptr->Size)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_str("Status", line, ptr->_status_str)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            string s;
            if (try_parse_str("SourceDateEpoch", line, s)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_str("SourceName", line, s)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_str("Source", line, s)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'U':{
            //URL appears to be an alias for Homepage?
            if (try_parse_str("URL", line, ptr->Homepage)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'V': {
            string s;
            if (try_parse_str("Version", line, s)) {
                parsing_desc = false;
                parsing_conf = false;
                if(upstream)
                    ptr->UpstreamVersion = s;
                else
                    ptr->InstalledVersion = s;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case ' ': {
            if (parsing_desc) {
                //newlines in descriptions are prefixed with a space, apparently
                utils::trim(ptr->Description);
                ptr->Description.append("\n");
                auto ld = string(line);
                //printf("Appending line to description for package %s\n 1: %s\n 2: %s\n", ptr->Package.c_str(), ptr->Description.c_str(), ld.c_str());
                utils::trim(ld);
                ptr->Description.append(ld);
                break;
            }
            if (parsing_conf) {
                utils::trim(ptr->Conffiles);
                ptr->Conffiles.append("\n");
                auto ld = string(line);
                utils::trim(ld);
                ptr->Conffiles.append(ld);
                break;
            }
        }
        NOT_RECOGNIZED:
        default: {
            auto dln = strlen(line);
            if (dln <= 2)
                return false;
            for (uint i = 0; i < dln; i++)
                if (line[i] != ' ' && line[i] != '\n' && line[i] != '\r') {
                    printf("BadChar: %d :: %2x :: %c\n", i, line[0], line[0]);
                    printf("Unhandled tag: %s\n", line);
                    printf("Last good line: %s\n", lastLine.c_str());
                    return true;
                }
            printf("UNKN: ");
            for (uint j = 0; j < dln; j++) {
                printf("%2x ", line[j]);
            }
            printf("\n");
            return false;
        }
    }
    lastLine = line;
    return true;
}

void opkg::InitializeRepositories() {
//TODO: This can most likely be parallelized to shorten the startup delay on multicore devices
    packages.clear();
    int pc = 0;
    char cbuf[4096]{};
    auto pk = make_shared<package>();
    for (const auto &f: fs::directory_iterator(OPKG_DB)) {
        printf("extracting archive %s\n", f.path().c_str());
        auto gzf = gzopen(f.path().c_str(), "rb");
        repositories.push_back(f.path().filename());
        int count = 0;
        //gzgets reads one line out of a gzipped file
        while (gzgets(gzf, cbuf, sizeof(cbuf)) != nullptr) {
            count++;
            if (!parse_line(pk, cbuf, false, true)) {     //if parse_line returns false, we're done parsing this package
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
    printf("Parsed %d packages\n", pc);

    //process status and info for installed packages
    auto statuspath = OPKG_LIB;
    statuspath += "/status";
    ifstream statusfile;
    statusfile.open(statuspath, ios::in);
    if(!statusfile.is_open())
        printf("fail opening status file %s\n", statuspath.c_str());

    pc = 0;
    for(string line; getline(statusfile, line);){
        parse_line(pk, line.c_str(), true, false);     //no need to do any logic here;
        pc++;                                   //parse_line will take care of updating extant packages
    }
    statusfile.close();

    printf("parsed %d status lines\n", pc);

    pc = 0;
    auto infopath = OPKG_LIB;
    infopath += "/info";
    int fc = 0;
    for (const auto &f: fs::directory_iterator(infopath)) {
        if (f.path().extension() == ".control") {
            fc++;
            ifstream cfile;
            cfile.open(f.path(), ios::in);
            if (!cfile.is_open()) {
                printf("ERROR! Failed to open control file %s\n", f.path().c_str());
                continue;
            }
            auto pname = f.path().filename().string().substr(0, f.path().filename().string().find_last_of('.'));
            auto pit = packages.find(pname);
            if (pit == packages.end()) {
                printf("ERROR! Could not match package %s to control file %s\n", pname.c_str(), f.path().c_str());
                continue;
            }
            pk = pit->second;
            for (string line; getline(cfile, line);) {
                pc++;
                parse_line(pk, line.c_str(), false, false);    //no need to update extant, we know what package this is from the filename
            }
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
    // Check if the line starts with "Removing package "
    size_t prefixPos = line.find("Removing package ");
    if (prefixPos != string::npos) {
        // Extract substring starting after "Removing package "
        string packagePart = line.substr(prefixPos + 17);
        // Find the first space to get the package name
        size_t spacePos = packagePart.find(' ');
        if (spacePos != string::npos) {
            uninstall_cache.insert(packagePart.substr(0, spacePos));
        }
        else {
            // If there's no space, the entire part is the package name
            uninstall_cache.insert(packagePart);
        }
    }
    else {
        // Check if the line ends with ", removing."
        size_t suffixPos = line.find(", removing.");
        if (suffixPos != string::npos) {
            // Extract substring up to the suffix
            string packagePart = line.substr(0, suffixPos);
            // Split into words and take the last one as the package name
            vector<string> parts;
            size_t start = 0;
            while (start < packagePart.size()) {
                size_t end = packagePart.find(' ', start);
                if (end == string::npos) {
                    end = packagePart.size();
                }
                if (end != start) { // Avoid empty tokens
                    parts.push_back(packagePart.substr(start, end - start));
                }
                start = end + 1;
            }
            if (!parts.empty()) {
                uninstall_cache.insert(parts.back());
            }
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
