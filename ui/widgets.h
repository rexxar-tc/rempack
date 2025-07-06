//
// Created by brant on 1/26/24.
//

#pragma once

#ifndef NDEBUG
//#define WIDGET_DEBUG
#endif

#include <list>
#include <utility>
#include "icons/icons_embed.h"
#include "utils.h"
#include <functional>
#include <any>
#include "text_helpers.h"

namespace widgets {
    constexpr remarkable_color toRColor(uint8_t r, uint8_t g, uint8_t b)
    {
        uint16_t r5 = (r * 31 + 127) / 255;  // Scale 0–255 to 0–31
        uint16_t g6 = (g * 63 + 127) / 255;  // Scale 0–255 to 0–63
        uint16_t b5 = (b * 31 + 127) / 255;  // Scale 0–255 to 0–31

        return (r5 << 11) | (g6 << 5) | b5;
    }

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

        void render() override;

        void undraw() override;

        void render_border() override;

        void render_inside_fill(float gray = 1.f);
        RoundCornerStyle style;
    protected:
        virtual void debugRender();
        uint16_t undraw_color = WHITE;
    };


}
