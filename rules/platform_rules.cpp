//
// Created by brant on 7/5/25.
//

#include "platform_rules.h"

namespace platform {
    void PlatformRules::sortPackages(vector<std::string> &strings) {
        std::sort(strings.begin(), strings.end());
    }

    std::vector<shared_ptr<package>> PlatformRules::checkSplashConflicts(const opkg &opkg, const shared_ptr<package> &pkg) {
        return {};
    }

    bool splashComparator(const std::string& a, const std::string& b) {
        auto isSplashscreen = [](const std::string& s) {
            return s.rfind("splashscreen-", 0) == 0;
        };

        auto extractGroupKey = [](const std::string& s) -> std::string {
            // Assume format: splashscreen-<type>-<xyz>
            size_t lastDash = s.rfind('-');
            if (lastDash != std::string::npos) {
                return s.substr(lastDash + 1);
            }
            return s;
        };

        bool aIsSplash = isSplashscreen(a);
        bool bIsSplash = isSplashscreen(b);

        if (aIsSplash && bIsSplash) {
            return extractGroupKey(a) < extractGroupKey(b);
        } else if (aIsSplash) {
            return "splashscreen" < b;
        } else if (bIsSplash) {
            return a < "splashscreen";
        } else {
            return a < b;
        }
    }

    void RemarkableRules::sortPackages(vector<std::string> &strings) {
        std::sort(strings.begin(), strings.end(), splashComparator);
    }

    bool hasSameSplashscreenPrefix(const std::string& a, const std::string& b) {
        auto extractPrefix = [](const std::string& s) -> std::string {
            if (s.rfind("splashscreen-", 0) != 0)
                return ""; // Not a splashscreen entry

            // Find position of second dash (after "splashscreen-")
            size_t firstDash = s.find('-', 0);              // always 12
            size_t secondDash = s.find('-', firstDash + 1); // dash after <type>

            if (secondDash == std::string::npos)
                return ""; // Malformed splashscreen entry

            return s.substr(0, secondDash + 1); // includes the trailing dash
        };

        return extractPrefix(a) == extractPrefix(b);
    }

    std::vector<shared_ptr<package>> RemarkableRules::checkSplashConflicts(const opkg &opkg, const shared_ptr<package> &pkg) {
        std::vector<shared_ptr<package>> ret {};
        for(const auto& pk : opkg.packages){
            if(!pk.second->IsInstalled())
                continue;
            if(!hasSameSplashscreenPrefix(pkg->Package, pk.second->Package))
                continue;
            ret.push_back(pk.second);
        }
        return ret;
    }
} // platform