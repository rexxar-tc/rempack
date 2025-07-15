//
// Created by brant on 7/3/25.
//

#pragma once

#include <string>
#include <map>
#include <unordered_set>
namespace widgets{
    struct FilterOptions{
        bool Installed;
        bool Upgradable;
        bool NotInstalled;
        bool SearchDescription;
        bool SearchHidden;
        bool groupSplash;
        std::map<std::string, bool> Repos;
        std::map<std::string, bool> Licenses;
        std::unordered_set<std::string> Sections;
    };

}