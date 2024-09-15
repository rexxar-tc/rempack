//
// Created by brant on 2/5/24.
//

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

    //inline std::string wrap_string(std::string s, short fontSize, int &outHeight, int widthLimit, int heightLimit = -1){   }

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