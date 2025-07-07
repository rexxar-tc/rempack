//
// Created by brant on 7/5/25.
//

#include "platform_rules.h"



namespace platform {
    std::vector<shared_ptr<package>> PlatformRules::checkSplashConflicts(const opkg &opkg, const shared_ptr<package> &pkg) {
        return {};
    }

    bool PlatformRules::splashscreenComparator(const shared_ptr<widgets::ListBox::ListItem> &a, const shared_ptr<widgets::ListBox::ListItem> &b) {
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

        string astr = a->key;
        string bstr = b->key;
        bool aIsSplash = isSplashscreen(astr);
        bool bIsSplash = isSplashscreen(bstr);

        if (aIsSplash && bIsSplash) {
            return extractGroupKey(astr) < extractGroupKey(bstr);
        } else if (aIsSplash) {
            return "splashscreen" < bstr;
        } else if (bIsSplash) {
            return astr < "splashscreen";
        } else {
            return astr < bstr;
        }
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