//
// Created by brant on 7/3/25.
//

#pragma once
#include "widgets.h"
#include "input.h"
namespace widgets{
    //same as ui::TextInput except it draws fancy rounded corners
    class RoundedTextInput: public ui::TextInput{
    public:
        RoundCornerStyle style;
        shared_ptr<RoundCornerWidget> border;
        RoundedTextInput(int x, int y, int w, int h, RoundCornerStyle style, string text = ""): ui::TextInput(x,y,w,h,std::move(text)){
            //TODO: this is so bad
            //TODO: style sheets
            ui::TextInput::style.valign = ui::Style::MIDDLE;
            ui::TextInput::style.justify = ui::Style::LEFT;
            this->style = style;
            border = make_shared<RoundCornerWidget>(x,y,w,h,style);
            children.push_back(border);
        }

        void on_reflow()override;

        void render() override;
    };
}