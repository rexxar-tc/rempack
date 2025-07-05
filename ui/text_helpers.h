//
// Created by brant on 2/5/24.
//

//#define DISABLE_TEXT_HELPER
#ifdef DISABLE_TEXT_HELPER
#include "text/text_helpers.h"
#else
#pragma once

#include <rmkit.h>

namespace utils{
    inline std::tuple<int,int> measure_string(std::string s, short fontSize){
        auto v = stbtext::get_text_size(s,fontSize);
        return make_tuple(v.w, v.h);
    }

    inline int line_height(ui::Style style = ui::Style::DEFAULT){
        return stbtext::get_line_height(style.font_size);
    }

    inline std::vector<std::string> wrap_string(const std::string &s, int widthLimit, short fontSize) {
        std::vector<std::string> res;
        std::stringstream ss;
        auto sw = stbtext::get_text_size(" ", fontSize).w;
        auto lines = split_lines(s);
        int x = 0;
        for(const auto &line : lines) {
            auto tokens = split(line, ' ');
            for (std::string &tk: tokens) {
                auto token = tk;
                auto v = stbtext::get_text_size(token, fontSize);
                v.w += sw;
                if(x + v.w < widthLimit) {
                    x += v.w;
                    ss << token << ' ';
                }
                else{
                    //hard wrap long tokens
                    if(v.w > widthLimit){
                        int soffset = 0;
                        for(int i = 1; i <= token.size(); i++){
                            auto sstr = token.substr(soffset, i - soffset);
                            auto sv = stbtext::get_text_size(sstr, fontSize);
                            if(x + sv.w < widthLimit && i < token.size())
                                continue;
                            soffset = i;
                            x = 0;
                            ss << sstr;
                            res.emplace_back(ss.str());
                            ss.str("");
                        }
                    }
                    else {
                        res.emplace_back(ss.str());
                        ss.str("");
                        x = v.w;
                        ss << token << ' ';
                    }
                }
            }
            res.emplace_back(ss.str());
            ss.str("");
        }
        return res;
    }

    inline std::string clip_string(std::string s, int w, int h, short fontSize){
        std::stringstream ss;
        auto line_height = stbtext::get_line_height(fontSize);
        auto sw = stbtext::get_text_size(" ", fontSize).w;
        auto lines = split_lines(s);
        int x = 0;
        int y = 0;
        for(const auto &line : lines) {
            y += line_height;
            auto tokens = split(line, ' ');
            for (std::string &tk: tokens) {
                auto token = tk;
                auto v = stbtext::get_text_size(token, fontSize);
                v.w += sw;
                if(x + v.w < w)
                    x += v.w;
                else{
                    if(y + line_height > h)
                        return ss.str();
                    y += line_height;
                    x = v.w;
                }
                ss << token << ' ';
            }
            ss << std::endl;
        }
        return s;
    }
}
#endif