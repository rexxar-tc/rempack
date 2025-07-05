//
// Created by brant on 7/5/25.
//

#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <algorithm>
#include "opkg.h"

namespace platform {

    /*
     * Override specific behaviors based on platform details
     */
struct PlatformRules {
    /**
     * Sorts packages such that 'splashscreen-<type>-xyz' are grouped together
     * @param strings
     */
    virtual void sortPackages(std::vector<std::string>& strings);

    /**
     * Checks for conflicts between a splashscreen package and currently installed packages
     * @param opkg reference to the opkg instance
     * @param pkg the package to be installed
     * @return a list of packages that conflict, or an empty list if there are no conflicts
     */
    virtual std::vector<shared_ptr<package>> checkSplashConflicts(const opkg& opkg, const shared_ptr<package>& pkg);
};
struct RemarkableRules: public PlatformRules{

    void sortPackages(std::vector<std::string> &strings) override;

    std::vector<shared_ptr<package>> checkSplashConflicts(const opkg &opkg, const shared_ptr<package> &pkg) override;
};

//#ifdef REMARKABLE
    inline RemarkableRules rules;
//#else
//    inline PlatformRules rules;
//#endif
} // platform
