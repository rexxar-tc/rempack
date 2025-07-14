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
        RoundedTextInput(int x, int y, int w, int h, RoundCornerStyle style, string text = "");

        void on_reflow()override;

        void render() override;
    };
}