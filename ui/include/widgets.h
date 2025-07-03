//
// Created by brant on 1/26/24.
//

#pragma once

#include <list>
#include <utility>
#include "icons/icons_embed.h"
#include "utils.h"
#include <functional>
#include <any>
#include "text_helpers.h"

namespace widgets {
    /*
     * ╭----╮
     * |    |
     * ╰----╯
     *
     * Draws all four rounded corners as described by the center and radius of the top-left corner,
     * and the offset to the bottom-right corner
     *
     * Optionally, you can specify a color for the border, its thickness, and a gradient
     * Thickness expands outward from the centerpoint, and the gradient is a sigmoid function (utils::sigmoid)
     *
     * colors are specified as a float in the range (0,1) where 0 is black and 1 is white (see color::from_float)
     *
     * This is an adaptation of the circle outline algorithm at framebuffer::draw_circle_outline
     * The gradient is accomplished by drawing multiple 1px arcs with diminishing color in 1px steps away from center
     *
     * We only actually compute one eighth of the circle and mirror it to all the other octants
     */

    void drawRoundedCorners(int x0, int y0, int ox, int oy, int radius, framebuffer::FB *fb,
                                   float grayfColor = 0, uint stroke = 1, bool gradient = false,
                                   float grayfendColor = 1,
                                   float expA = -20.f, float coefB = 8, float alphaMask = 0.9f);

    //draws a box with rounded corners with some style options
    //calls into drawRoundedCorners first, then draws the lines connecting the arcs
    //gradient is based on a sigmoid function (see utils::sigmoid)
    //colors are specified as a float in the range (0,1) where 0 is black and 1 is white (see color::from_float)
    inline void drawRoundedBox(int x0, int y0, int w, int h, int radius, framebuffer::FB *fb,
                               int stroke = 1, float grayfColor = 0, int shrink = 0, bool gradient = false,
                               float grayfendColor = 1,
                               float expA = -20.f, float coefB = 8, float alphaThreshold = 0.9f);

class EventButton : public ui::Button{
    public:
    EventButton(int x, int y, int w, int h, string text = "") : Button(x, y, w, h, text){}

    PLS_DEFINE_SIGNAL(BUTTON_EVENT, void*);

    class BUTTON_EVENTS {
    public:
        BUTTON_EVENT clicked;
    };

    BUTTON_EVENTS events;
    void on_mouse_click(input::SynMotionEvent &ev) override;

    void disable();

    void enable();

    void render() override;
    protected:
    bool enabled = true;
};

//basically a reimplementation of ui::Button with a clickable image instead of text
    class ImageButton : public EventButton {
    public:
        ImageButton(int x, int y, int w, int h, icons::Icon icon) : EventButton(x, y, w, h, ""){
                pixmap = make_shared<ui::Pixmap>(x, y, w, h, icon);
                pixmap->alpha = WHITE;
                children.push_back(pixmap);
            };


        void render() override;

        void on_reflow() override;

        void on_mouse_click(input::SynMotionEvent &ev) override;


    private:
        shared_ptr<ui::Pixmap> pixmap;
    };

    //TODO: style sheets
    struct RoundCornerStyle {
    public:
        int cornerRadius;
        int borderThickness;
        float startColor;
        float endColor;
        bool gradient;
        //shrink the border by this many pixels. Generally, set this equal to cornerRadius
        int inset;
        float expA;
        float expB;

        RoundCornerStyle() {
            cornerRadius = 10;
            borderThickness = 10;
            startColor = 0;
            endColor = 1;
            gradient = true;
            inset = 8;
            expA = -9.f;
            expB = 1.f;
        }
    };


    class RoundCornerWidget : public ui::Widget {
    public:
        RoundCornerWidget(int x, int y, int w, int h, RoundCornerStyle style) : ui::Widget(x, y, w, h){};
        RoundCornerStyle style;
            uint16_t undraw_color = WHITE;

        void undraw() override;

        void render_border() override;

        void render_inside_fill(float gray = 1.f);
    };

    class RoundImageButton : public ImageButton{
    public:

        shared_ptr<RoundCornerWidget> border;
        RoundImageButton(int x, int y, int w, int h, icons::Icon icon, RoundCornerStyle style): ImageButton(x,y,w,h,icon){
            border = make_shared<RoundCornerWidget>(x,y,w,h,style);
            children.push_back(border);
        }
        void on_reflow()override;
    };

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

   class LabeledRangeInput : public ui::Widget {
    public:
        enum LabelPosition {
            LEFT, TOP
        };

        LabeledRangeInput(int x, int y, int w, int h, string text = "", LabelPosition pos = LEFT, int padding = 5)
                : ui::Widget(x, y, w, h) {
            if (!text.empty()) {
                label = make_shared<ui::Text>(x, y, w, (h / 2) - padding, text);
                children.push_back(label);
                y += h / 2;
                h = h / 2 - padding;
            }
            range = make_shared<ui::RangeInput>(x, y, w, h);

            children.push_back(range);
            w = range->x + range->w - x;
            h = range->y + range->h - y;
        }

        void mark_redraw() override;


        shared_ptr<ui::RangeInput> range;
    private:
        shared_ptr<ui::Text> label = nullptr;
    };

}
