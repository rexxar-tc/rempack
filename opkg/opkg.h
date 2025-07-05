//
// Created by brant on 2/2/24.
//

#pragma once

#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <ctime>
#include <unordered_set>
#include <functional>
#include <iostream>
#include <array>
#include <unistd.h>

//set to enforce the --noaction flag on all opkg transactions
#define OPKG_FORCE_NOACTION false

using namespace std;
struct package{
    enum InstallState{
        InstallError,
        NotInstalled,
        Installed,
    };
    string Package;
    string Description;
    string Homepage;
    string InstalledVersion;
    string UpstreamVersion;
    string Section;
    string Maintainer;
    string License;
    string Architecture;
    string Filename;
    string SHA256sum;
    uint Size;
    string Repo;
    string _depends_str;
    vector<shared_ptr<package>> Depends;
    vector<shared_ptr<package>> Dependents;
    string _recommends_str;
    vector<shared_ptr<package>> Recommends;
    bool autoInstalled;
    long installTime;
    string _status_str;
    [[nodiscard]] bool IsInstalled() const {return State == Installed;}
    InstallState State;
    string Alternatives;
    string _conflicts_str;
    vector<shared_ptr<package>> Conflicts;
    string _replaces_str;
    vector<shared_ptr<package>> Replaces;
    string Conffiles;
    string _provides_str;
    vector<shared_ptr<package>> Provides;
    vector<shared_ptr<package>> ProvidedBy;
    bool Essential;
    bool Virtual;
    //TODO: this is not great
    [[nodiscard]] bool Upgradable() const{ return IsInstalled() && InstalledVersion != UpstreamVersion; }
};
class opkg {
public:
    static inline opkg* Instance;
    static int Install(const vector<shared_ptr<package>> &targets, const function<void(const string &)> &lineCallback, const std::string& args = "");
    static int Uninstall(const vector<shared_ptr<package>> &targets, const function<void(const string &)> &lineCallback, const std::string& args = "");
    static int UpdateRepos(const function<void(const string &)> &lineCallback);
    static string DownloadPackage(const shared_ptr<package>& pkg, const function<void(const string &)> &lineCallback, const string& targetPath = "");
    void LoadSections(vector<string> *categories){ LoadSections(categories,vector<string>()); }
    void LoadSections(vector<string> *categories, vector<string> excludeRepos);
    void LoadPackages(vector<string> *pVector){ LoadPackages(pVector, vector<string>()); }
    void LoadPackages(vector<string> *pVector, vector<string> excludeRepos);
    void InitializeRepositories();
    void link_dependencies();
    static vector<uint8_t> getCachedSplashscreen(const shared_ptr<package>& pkg);
    static bool isPackageCached(const shared_ptr<package>& pkg);
    map<string, shared_ptr<package>> packages;
    vector<string> repositories;
    vector<string> sections;
    map<string,unordered_set<string>> sections_by_repo;
    map<string,vector<shared_ptr<package>>> virtual_packages;
    static string FormatPackage(const shared_ptr<package>& package);
    static string formatDependencyTree(const shared_ptr<package>& pkg, bool excludeInstalled);
    int ComputeUninstall(const vector<shared_ptr<package>>& targets, bool includeDependencies, vector<shared_ptr<package>> *out_result);
private:
    bool parse_line(shared_ptr<package> &ptr, const char *line, bool update, bool upstream);
    bool split_str_and_find(const string& children_str, vector<shared_ptr<package>> &field);
    void update_states();
    void update_lists();
};


