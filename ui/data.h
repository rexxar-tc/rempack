//
// Created by brant on 7/3/25.
//

#pragma once

#include <string>
#include <map>
#include <unordered_set>

namespace widgets {
    struct MenuData {
        std::unordered_set <std::string> PendingInstall;
        std::unordered_set <std::string> PendingRemove;
        bool Cronch;
        int FontSize;
    };
    struct FilterOptions {
        bool Installed;
        bool Upgradable;
        bool NotInstalled;
        bool SearchDescription;
        std::map<std::string, bool> Repos;
        std::map<std::string, bool> Licenses;
    };
}