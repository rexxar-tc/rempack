//
// Created by brant on 1/26/24.
//

#pragma once

#include <rmkit.h>
#include <list>
#include <utility>
#include "../assets/icons/icons_embed.h"
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

    inline void drawRoundedCorners(int x0, int y0, int ox, int oy, int radius, framebuffer::FB *fb,
                                   float grayfColor = 0, uint stroke = 1, bool gradient = false,
                                   float grayfendColor = 1,
                                   float expA = -20.f, float coefB = 8, float alphaMask = 0.9f) {
        int x = 0;
        int y = radius;
        int d = -(radius >> 1);
        int w = stroke;
        int h = stroke;
        auto color = color::from_float(grayfColor);

        //since we use _draw_rect_fast, we need to manually mark the area dirty
        fb->update_dirty(fb->dirty_area, x0 - radius - stroke, y0 - radius - stroke);
        fb->update_dirty(fb->dirty_area, x0 + ox + radius + stroke, y0 + oy + radius + stroke);
        fb->waveform_mode = WAVEFORM_MODE_GC16;

        if (!gradient) {
            while (x <= y) {
                //q4
                fb->_draw_rect_fast(-x + x0 - stroke, -y + y0 - stroke, w, h, color);
                fb->_draw_rect_fast(-y + x0 - stroke, -x + y0 - stroke, w, h, color);
                //q1
                fb->_draw_rect_fast(x + x0 + ox, -y + y0 - stroke, w, h, color);
                fb->_draw_rect_fast(y + x0 + ox, -x + y0 - stroke, w, h, color);
                //q2
                fb->_draw_rect_fast(x + x0 + ox, y + y0 + oy, w, h, color);
                fb->_draw_rect_fast(y + x0 + ox, x + y0 + oy, w, h, color);
                //q3
                fb->_draw_rect_fast(-x + x0 - stroke, y + y0 + oy, w, h, color);
                fb->_draw_rect_fast(-y + x0 - stroke, x + y0 + oy, w, h, color);

                if (d <= 0) {
                    x++;
                    d += x;
                } else {
                    y--;
                    d -= y;
                }
            }
        } else {
            auto dc = abs(grayfendColor - grayfColor) / (float) stroke;
            for (uint si = 0; si <= stroke; si++) {
                auto fc = utils::sigmoid(grayfColor + dc * si, expA, coefB);
                if(fc < alphaMask)
                    drawRoundedCorners(x0, y0, ox, oy, radius + si, fb, fc);
            }
        }
    }

    //draws a box with rounded corners with some style options
    //calls into drawRoundedCorners first, then draws the lines connecting the arcs
    //gradient is based on a sigmoid function (see utils::sigmoid)
    //colors are specified as a float in the range (0,1) where 0 is black and 1 is white (see color::from_float)
    inline void drawRoundedBox(int x0, int y0, int w, int h, int radius, framebuffer::FB *fb,
                               int stroke = 1, float grayfColor = 0, int shrink = 0, bool gradient = false,
                               float grayfendColor = 1,
                               float expA = -20.f, float coefB = 8, float alphaThreshold = 0.9f) {
        int sx = x0 + shrink;
        int sy = y0 + shrink;
        int dx = w - (2 * shrink);
        int dy = h - (2 * shrink);

        if (!gradient) {
            auto color = color::from_float(grayfColor);
            drawRoundedCorners(sx, sy, dx, dy, radius, fb, grayfColor, stroke);
            fb->_draw_rect_fast(sx - stroke - radius, sy, stroke, dy, color);
            fb->_draw_rect_fast(sx + dx + radius, sy, stroke, dy, color);
            fb->_draw_rect_fast(sx, sy - stroke - radius, dx, stroke, color);
            fb->_draw_rect_fast(sx, sy + dy + radius, dx, stroke, color);
        } else {
            drawRoundedCorners(sx, sy, dx, dy, radius, fb, grayfColor, stroke, gradient, grayfendColor, expA, coefB, alphaThreshold);
            float dc = abs(grayfendColor - grayfColor) / (float) stroke;
            for (int i = 0; i <= stroke; i++) {
                auto fc = utils::sigmoid(grayfColor + (dc * i), expA, coefB);
                if(fc>=alphaThreshold)
                    continue;   //don't break, the curve may change later in the stroke
                auto color = color::from_float(fc);
                //left
                fb->_draw_rect_fast(sx - i - radius - 1, sy, 1, dy, color);
                //right
                fb->_draw_rect_fast(sx + dx + radius + i, sy, 1, dy, color);
                //top
                fb->_draw_rect_fast(sx, sy - i - radius - 1, dx, 1, color);
                //bottom
                fb->_draw_rect_fast(sx, sy + dy + i + radius, dx, 1, color);
            }
        }
    }

class EventButton : public ui::Button{
    public:
    EventButton(int x, int y, int w, int h, string text = "") : Button(x, y, w, h, text){}

    PLS_DEFINE_SIGNAL(BUTTON_EVENT, void*);

    class BUTTON_EVENTS {
    public:
        BUTTON_EVENT clicked;
    };

    BUTTON_EVENTS events;
    void on_mouse_click(input::SynMotionEvent &ev) override {
        ev.stop_propagation();
        if(!enabled)
            return;
        events.clicked();
        mark_redraw();
    }

    void disable() {
        enabled = false;
        mark_redraw();
    }

    void enable() {
        enabled = true;
        mark_redraw();
    }

    void render() override{
        ui::Button::render();
        if(!enabled) {
            fb->draw_rect(x, y, w, h, color::GRAY_12, true);
        }
        //fb->draw_rect(x,y,w,h,BLACK,false);
    }
    protected:
    bool enabled = true;
};

//basically a reimplementation of ui::Button with a clickable image instead of text
    class ImageButton : public EventButton {
    public:
        ImageButton(int x, int y, int w, int h, icons::Icon icon) : EventButton(x, y, w, h, "") {
            pixmap = make_shared<ui::Pixmap>(x, y, w, h, icon);
            pixmap->alpha = WHITE;
            children.push_back(pixmap);
        }


        void render() override{
            EventButton::render();
            //fb->waveform_mode = WAVEFORM_MODE_GC16;
            pixmap->render();
        }

        void on_reflow() override {
            //pixmap->undraw();
            auto dw = min(w,h);
            auto dx = x+(w/2) - (dw/2);
            pixmap->set_coords(dx, y, dw, dw);
            //pixmap->icon.width  = dw;
            //pixmap->icon.height = dw;
            //util::resize_image(pixmap->icon.image, dw, dw, 20);
            pixmap->on_reflow();
            pixmap->mark_redraw();
        }

        void on_mouse_click(input::SynMotionEvent &ev) override {
            ev.stop_propagation();
            if(!enabled)
                return;
            EventButton::on_mouse_click(ev);
        }


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


    class RoundCornerWidget : public rmlib::Widget {
    public:
        RoundCornerWidget(int x, int y, int w, int h, RoundCornerStyle style) : rmlib::Widget(x, y, w, h) {
            this->style = style;
        };
        RoundCornerStyle style;
            uint16_t undraw_color = WHITE;

            //TODO: this still isn't quite right
        void undraw() override {
                return;
            //top
            fb->draw_rect(x + style.inset - style.cornerRadius - style.borderThickness,
                          y + style.inset - style.cornerRadius - style.borderThickness,
                          w - style.inset + style.cornerRadius + style.borderThickness,
                          style.borderThickness,
                          undraw_color, true);
            //bottom
            fb->draw_rect(x + style.inset - style.cornerRadius - style.borderThickness,
                          y + h - style.inset + style.cornerRadius,
                          w - style.inset + style.cornerRadius + style.borderThickness,
                          style.borderThickness,
                          undraw_color, true);
            //left
            fb->draw_rect(x + style.inset - style.cornerRadius - style.borderThickness,
                          y + style.inset - style.cornerRadius - style.borderThickness,
                          style.borderThickness,
                          h - style.inset + style.cornerRadius + style.borderThickness,
                          undraw_color, true);
            //right
            fb->draw_rect(x + w - style.inset + style.cornerRadius,
                          y + style.inset - style.cornerRadius - style.borderThickness,
                          style.borderThickness,
                          h - style.inset + style.cornerRadius + style.borderThickness,
                          undraw_color, true);
        }

        void render_border() override {
            drawRoundedBox(x, y, w, h, style.cornerRadius, fb, style.borderThickness,
                           style.startColor, style.inset, style.gradient, style.endColor,
                           style.expA, style.expB);
        }

        void render_inside_fill(float gray = 1.f){
            //draw a rounded box to fill the awkward space between the border and inner content
            drawRoundedBox(x, y, w, h, style.cornerRadius, fb, style.cornerRadius,
                           gray, style.inset + (style.cornerRadius), false,1,1,1,2.f); //extra junk here because C++ doesn't
                                                                                     //support named parameters and I need to change the alpha
            //draw a rectangle to cover the rest of the inner area
            fb->draw_rect(x + style.inset, y + style.inset,
                          w - style.inset - style.inset, h - style.inset - style.inset,
                          color::from_float(gray), true);
        }
    };

    class RoundImageButton : public ImageButton{
    public:

        shared_ptr<RoundCornerWidget> border;
        RoundImageButton(int x, int y, int w, int h, icons::Icon icon, RoundCornerStyle style): ImageButton(x,y,w,h,icon){
            border = make_shared<RoundCornerWidget>(x,y,w,h,style);
            children.push_back(border);
        }
        void on_reflow()override{
            border->set_coords(x,y,w,h);
            border->mark_redraw();
            ImageButton::on_reflow();
        }
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
    void undraw() override {
        auto sx = x - style.cornerRadius - style.borderThickness + style.inset;
        auto sy = y - style.cornerRadius - style.borderThickness + style.inset;
        auto dx = sx + w + style.cornerRadius + style.borderThickness + style.inset;
        auto dy = sy + h + style.cornerRadius + style.borderThickness + style.inset;
        fb->draw_rect(sx, sy, dx, dy, WHITE, true);
    }
     
    void render() override {
        fb->waveform_mode = WAVEFORM_MODE_GC16;
        ui::TextInput::render();
    }

    void on_reflow()override{
        border->set_coords(x,y,w,h);
        border->mark_redraw();
    }

    void render_border() override{
        border->style = style;
        //stop parent class from rendering its border
    }
};

   class LabeledRangeInput : public rmlib::Widget {
    public:
        enum LabelPosition {
            LEFT, TOP
        };

        LabeledRangeInput(int x, int y, int w, int h, string text = "", LabelPosition pos = LEFT, int padding = 5)
                : rmlib::Widget(x, y, w, h) {
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

        void mark_redraw() override{
            range->mark_redraw();
            if(label != nullptr)
                label->mark_redraw();
        }


        shared_ptr<ui::RangeInput> range;
    private:
        shared_ptr<ui::Text> label = nullptr;
    };

}
