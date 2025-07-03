//
// Created by brant on 2/2/24.
//

#pragma once

#include "widgets.h"
#include "input/input.h"

namespace widgets{
    class GradientGizmo : public ui::Widget {
    public:
        GradientGizmo(int x, int y, int w, int h, float expA, float coefB, bool border = true) : Widget(x, y, w, h) {
            this->expA = expA;
            this->coefB = coefB;
            drawBorder = border;
        }

        float expA;
        float coefB;
        bool drawBorder;

        void render_border() override {
            if (drawBorder)
                fb->draw_rect(x - 1, y - 1, w + 2, h + 2, BLACK, false);
        }

        void undraw() override {
            fb->draw_rect(x, y, w, h, WHITE, true);
        }

        void render() override {
            float dc = 1.f / w;
            for (int i = 0; i <= w; i++) {
                fb->_draw_rect_fast(x + i, y, 1, h, color::from_float(utils::sigmoid(dc * i, expA, coefB)));
            }
            fb->waveform_mode = WAVEFORM_MODE_GC16;
            fb->dither = framebuffer::DITHER::BLUE_NOISE_16;
        }
    };


    class RoundCornerEditor : public ui::Widget {
    public:
        RoundCornerEditor(int x, int y, int w, int h, RoundedTextInput *target) : ui::Widget(x,y,w,h) {
            this->target = target;
            this->style= target->style;
            int padding = 20;
            //add a few gradient previews
            auto _x = x;
            int gh = 200;
            auto g1 = make_shared<GradientGizmo>(x, y, 5, gh, style.expA, style.expB, true);
            x += g1->w + padding;
            auto g2 = make_shared<GradientGizmo>(x, y, 10, gh, style.expA, style.expB, true);
            x += g2->w + padding;
            auto g3 = make_shared<GradientGizmo>(x, y, 50, gh, style.expA, style.expB, true);
            x += g3->w + padding;
            auto g4 = make_shared<GradientGizmo>(x, y, 100, gh, style.expA, style.expB, true);
            x += g4->w + padding;
            auto g5 = make_shared<GradientGizmo>(x, y, 500, gh, style.expA, style.expB, true);
            x = _x;
            y += g1->h + padding;
            gradients.push_back(g1);
            gradients.push_back(g2);
            gradients.push_back(g3);
            gradients.push_back(g4);
            gradients.push_back(g5);
            children.push_back(g1);
            children.push_back(g2);
            children.push_back(g3);
            children.push_back(g4);
            children.push_back(g5);

            int lineHeight = 100;
            inputA = make_shared<LabeledRangeInput>(x, y, w, lineHeight, "Exp");
            y += lineHeight + padding;
            inputA->range->set_range(-50, 0);
            inputA->range->set_value(style.expA);
            inputA->range->events.done += PLS_DELEGATE(this->updateSliderA);
            children.push_back(inputA);
            inputB = make_shared<LabeledRangeInput>(x, y, w, lineHeight, "Coef");
            y += lineHeight + padding;
            inputB->range->set_range(0, 20);
            inputB->range->set_value(style.expB);
            inputB->range->events.done += PLS_DELEGATE(this->updateSliderB);
            children.push_back(inputB);
            inputT = make_shared<LabeledRangeInput>(x, y, w, lineHeight, "THICC");
            y += lineHeight + padding;
            inputT->range->set_range(0, 50);
            inputT->range->set_value(style.borderThickness);
            inputT->range->events.done += PLS_DELEGATE(this->updateSliderT);
            children.push_back(inputT);
        }

        void update_values() {
            for (auto &g: gradients) {
                g->expA = target->style.expA;
                g->coefB = target->style.expB;
                g->mark_redraw();
            }
        }

    private:
        RoundedTextInput *target;
        RoundCornerStyle style;
        std::vector<shared_ptr<GradientGizmo>> gradients;
        shared_ptr<LabeledRangeInput> inputA, inputB, inputT, inputS, inputEnd;
        shared_ptr<ui::ToggleButton> gradientSw;

        void updateSliderA(float p) {
            target->style.expA = inputA->range->get_value();
            update_values();
            target->mark_redraw();
        }

        void updateSliderB(float p) {
            target->style.expB = inputB->range->get_value();
            update_values();
            target->mark_redraw();
        }

        void updateSliderT(float p) {
            target->style.borderThickness = inputT->range->get_value();
            update_values();
            target->mark_redraw();
        }

        void updateSliderS(float p) {
            target->style.startColor = p;
            inputS->mark_redraw();
            update_values();
        }

        void updateSliderE(float p) {
            target->style.endColor = p;
            inputEnd->mark_redraw();
            update_values();
        }
    };
}