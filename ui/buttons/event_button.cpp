//
// Created by brant on 7/3/25.
//
#include "event_button.h"
#include "../debug/debug_widgets.h"
#include <utility>
namespace widgets {
    EventButton::EventButton(int x, int y, int w, int h, string text, RoundCornerStyle style) : ui::Button(x, y, w, h, text), border_style(style) {
        x_padding = 0;
        y_padding = 0;
        border_style = style;
        border = make_shared<RoundCornerWidget>(x, y, w, h, style);
        children.push_back(border);

    }

    EventButton::EventButton(int x, int y, int w, int h, RoundCornerStyle style) : EventButton(x, y, w, h, "", style) {
    }

    EventButton::EventButton(int x, int y, int w, int h, string text) : Button(x, y, w, h, text) {
        x_padding = 0;
        y_padding = 0;
        //  border = make_shared<RoundCornerWidget>(x,y,w,h,widgets::LightButtonStyle());
        //  border->visible = false;
    }

    void EventButton::mark_redraw() {
        if (border)
            border->mark_redraw();
        Widget::mark_redraw();
    }

    void EventButton::on_mouse_click(input::SynMotionEvent &ev) {
        ev.stop_propagation();
        if (!enabled)
            return;
        events.clicked();
        mark_redraw();
    }

    void EventButton::disable() {
        enabled = false;
        if (border && borderEnabled)
            border->style.startColor = 3.0 / 31.0f;
        mark_redraw();
    }

    void EventButton::enable() {
        enabled = true;

        if (border && borderEnabled)
            border->style.startColor = border_style.startColor;
        mark_redraw();
    }

    void EventButton::render_border() {
        if (border && borderEnabled)
            border->render_border();
        else
            ui::Widget::render_border();
    }

    void EventButton::render() {
        ui::Button::render();
        if (!text.empty())
            textWidget->render();

        if (!enabled)
            fb->draw_rect_alpha(x, y, w, h, color::GRAY_12, 127);

        //fb->draw_rect(x,y,w,h,BLACK,false);
#ifdef WIDGET_DEBUG
        this->debugRender();
#endif
    }

    void EventButton::undraw() {
        if (border && borderEnabled)
            border->render_inside_fill();
    }

    void EventButton::show() {
        if (border && borderEnabled)
            border->show();
        Widget::show();
    }

    void EventButton::hide() {
        if (border && borderEnabled)
            border->hide();
        Widget::hide();
    }

    void EventButton::enableBorder(bool enable) {
        if (!border) {
            borderEnabled = false;
            return;
        }
        borderEnabled = enable;
        if (enable)
            border->show();
        else
            border->hide();
    }

    void EventButton::debugRender() {
        fb->draw_rect(x, y, w, h, toRColor(128, 0, 128), false);
        auto t = textWidget;
        fb->draw_rect(t->x + 2, t->y + 2, t->w - 4, t->h - 4, toRColor(0, 255, 128), false);
    }

    void EventButton::on_reflow() {
        if (border && borderEnabled) {
            border->set_coords(x, y, w, h);
            border->on_reflow();
        }
        Rect::on_reflow();
    }

    bool EventButton::is_enabled() {
        return enabled;
    }
}