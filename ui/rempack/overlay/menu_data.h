//
// Created by brant on 7/3/25.
//

#pragma once
#include <unordered_set>
#include <string>

namespace widgets{
    struct MenuData{
        std::unordered_set<std::string> PendingInstall;
        std::unordered_set<std::string> PendingRemove;
        bool Cronch;
        int FontSize;
    };
}