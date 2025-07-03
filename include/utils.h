//
// Created by brant on 1/26/24.
//

#pragma once

#include <cmath>
#include <string>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <cstddef>
#include <cstdint>

using namespace std;

namespace utils{
    inline float sigmoid(float t, float a, float b){
        return 1 / (1 + std::exp(a * t + b));
    }

    // trim from start (in place)
    inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    // trim from end (in place)
    inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    inline void trim(std::string &s){
        ltrim(s);
        rtrim(s);
    }

    inline string stringifySize(uint64_t bytes)
    {
        const string suffix[] = {"B", "KB", "MB", "GB", "TB"};
        char length = sizeof(suffix);

        int i = 0;
        double dblBytes = bytes;

        if (bytes > 1024) {
            for (i = 0; (bytes / 1024) > 0 && i<length-1; i++, bytes /= 1024)
                dblBytes = bytes / 1024.0;
        }

        stringstream ss;
        ss << std::fixed << std::setprecision(2) << dblBytes << suffix[i];
        return ss.str();
    }

    inline vector<string> split_str(const string &s, const char delimiter)
    {
        vector<string> splits;
        string _split;
        istringstream ss(s);
        while (getline(ss, _split, delimiter))
        {
            utils::ltrim(_split);
            splits.push_back(_split);
        }
        return splits;
    }
}
