//
// Created by brant on 2/17/24.
//

#pragma once

#include <rmkit.h>
#include "../opkg/opkg.h"
#include "include/rempack_widgets.h"
#include "include/overlay.h"

namespace widget_helpers{
        inline void format_deps_recursive(unordered_map<string, uint> &items, const shared_ptr<package> &pkg){
            if(pkg->IsInstalled())
                return;
            string size = pkg->Size == 0 ? "0" : utils::stringifySize(pkg->Size);
            stringstream ss;
            ss << pkg->Package << " - " << size;
            if(!items.emplace(ss.str(), pkg->Size).second)
                return;
            for(const auto &dpkg: pkg->Depends)
                format_deps_recursive(items, dpkg);
        }


}