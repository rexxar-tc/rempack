//
// Created by brant on 7/2/25.
//

#include "widgets.h"
#include <cstddef>

#include "rempack/rempack_widgets.h"
#include "buttons/buttons.h"
namespace widgets{
    void drawRoundedCorners(int x0, int y0, int ox, int oy, int radius, framebuffer::FB *fb,
                                   float grayfColor, uint stroke, bool gradient,
                                   float grayfendColor,
                                   float expA, float coefB, float alphaMask) {
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
    void drawRoundedBox(int x0, int y0, int w, int h, int radius, framebuffer::FB *fb,
                               int stroke, float grayfColor, int shrink, bool gradient,
                               float grayfendColor,
                               float expA, float coefB, float alphaThreshold) {
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



        //TODO: this still isn't quite right
        void RoundCornerWidget::undraw() {
            ui::Widget::undraw();
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

        void RoundCornerWidget::render_border() {
            drawRoundedBox(x, y, w, h, style.cornerRadius, fb, style.borderThickness,
                           style.startColor, style.inset, style.gradient, style.endColor,
                           style.expA, style.expB);
        }

        void RoundCornerWidget::render_inside_fill(float gray){
            //draw a rounded box to fill the awkward space between the border and inner content
            drawRoundedBox(x, y, w, h, style.cornerRadius, fb, style.cornerRadius,
                           gray, style.inset + (style.cornerRadius), false,1,1,1,2.f); //extra junk here because C++ doesn't
            //support named parameters and I need to change the alpha
            //draw a rectangle to cover the rest of the inner area
            fb->draw_rect(x + style.inset, y + style.inset,
                          w - style.inset - style.inset, h - style.inset - style.inset,
                          color::from_float(gray), true);
        }


    void DebuggableWidget::render() {
        Widget::render();
#ifdef WIDGET_DEBUG
        this->debugRender();
#endif
    }

    void DebuggableWidget::debugRender() {
        fb->draw_rect(this->x, this->y, this->w, this->h, toRColor(255,0,0), false);
    }
}