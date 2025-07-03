//
// Created by brant on 7/3/25.
//

#pragma once

#include <string>
#include <map>
namespace widgets{
    struct FilterOptions{
        bool Installed;
        bool Upgradable;
        bool NotInstalled;
        bool SearchDescription;
        std::map<std::string, bool> Repos;
        std::map<std::string, bool> Licenses;
    };

}