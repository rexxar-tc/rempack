//
// Created by brant on 7/1/25.
//

#include "opkg.h"
#include <filesystem>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include "zlib.h"
#include "utils.h"
#include <sstream>


#define  CONTAINS(x,z) ((x).find(z) != (x).end())
unordered_set<string> formatExcludeNames{"libc", "libgcc"};

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

        ss << "Installed size: " << utils::stringifySize(pk->Size) << endl;
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

