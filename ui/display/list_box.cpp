//
// Created by brant on 7/2/25.
//

#include "list_box.h"

typedef widgets::ListBox::ListItem ListItem;
namespace widgets {

    shared_ptr<ListItem> ListBox::add(const string &label, const std::any &object) {
        auto item = make_shared<ListItem>(label, object);
        item->_widget = make_shared<ui::Text>(x, y, w, itemHeight, label);
        //TODO: style sheets
        item->_widget->style.valign = ui::Style::MIDDLE;
        item->_widget->style.justify = ui::Style::LEFT;
        contents.push_back(item);
        events.added(item);
        this->mark_redraw();
        return item;
    }

    bool ListBox::remove(const string &label) {
        int i = 0;
        shared_ptr<ListItem> item = nullptr;
        for (; i < (int) contents.size(); i++) {
            auto ti = contents[i];
            if (label == ti->label) {
                item = ti;
                break;
            }
        }
        if (item != nullptr) {
            contents.erase(contents.begin() + i);
            events.removed(item);
            mark_redraw();
            return true;
        }
        return false;
    }

    void ListBox::removeAt(int index) {
        auto item = contents[index];
        auto w = item->_widget;
        contents.erase(contents.begin() + index);
        events.removed(item);
        mark_redraw();
    }

    void ListBox::clear() {
        contents.clear();
        mark_redraw();
    }

    void ListBox::on_reflow() {
        layout_buttons();
        trim_texts();
        mark_redraw();
    }

    void ListBox::layout_buttons() {
        int bx = x + padding;
        int by = y + h - itemHeight - padding;
        stringstream lss;
        lss << "[ " << maxPages() << "/" << maxPages() << " ]";
        auto [lbx, lby] = utils::measure_string(lss.str(), ui::Widget::style.font_size);
        auto lbw = lbx;// + padding;
        _pageLabel->set_coords(bx, by, lbw, itemHeight);
        _pageLabel->style.valign = ui::Style::VALIGN::BOTTOM;
        bx += lbw + padding;
        //this width calculation is wrong for very narrow windows
        //I can't be bothered to fix it right now
        auto buttonWidth = ((w - lbw - padding - padding) / 5);
        if (maxPages() <= 2)
            buttonWidth *= 2;
        buttonWidth = min(buttonWidth, 200);
        buttonWidth = max(buttonWidth, itemHeight);
        if (maxPages() > 2) {
            _navLL->set_coords(bx, by, buttonWidth, itemHeight);
            bx += buttonWidth + padding;
        }
        _navL->set_coords(bx, by, buttonWidth, itemHeight);
        bx += buttonWidth + padding;
        _navR->set_coords(bx, by, buttonWidth, itemHeight);
        bx += buttonWidth + padding;
        if (maxPages() > 2)
            _navRR->set_coords(bx, by, buttonWidth, itemHeight);
        _pageLabel->on_reflow();
        _navLL->on_reflow();
        _navL->on_reflow();
        _navR->on_reflow();
        _navRR->on_reflow();
        _pageLabel->mark_redraw();
        _navLL->mark_redraw();
        _navL->mark_redraw();
        _navR->mark_redraw();
        _navRR->mark_redraw();
    }

    void ListBox::undraw() {
//fb->draw_rect(this->x, this->y, this->w, this->h, WHITE, true);
        RoundCornerWidget::render_inside_fill();
        RoundCornerWidget::undraw();
    }

    void ListBox::render() {
        undraw();
        refresh_list();
        int sx = this->x + padding;
        int sy = this->y + padding;
        for (const auto &item: _currentView) {
            auto wi = item->_widget;
            wi->x = sx;
            wi->y = sy;
            wi->h = itemHeight;
            wi->w = w - padding - padding;
            wi->on_reflow();
            if (item->_selected) {
//TODO: style sheets
//item is selected, draw an effect
//I can't be bothered to make this configurable right now
                fb->draw_rect(wi->x, wi->y, wi->w, wi->h, color::GRAY_9, true);
            }
            wi->render();
            sy += itemHeight + padding;
        }
    }

//check the Y position relative to top of widget, divide by itemHeight
    void ListBox::on_mouse_click(input::SynMotionEvent &ev) {
//ev.stop_propagation();
        if (!selectable)
            return;
        auto hgt = itemHeight + padding;
        auto sy = ev.y - this->y;
        auto shgt = sy / hgt;
        int idx = floor(shgt);
//printf("Click at %d,%d: computed offset %d: displayed %d\n", ev.x, ev.y, idx, displayed_items());
        if (idx >= (int) _currentView.size())
            return;
        selectIndex(idx);
        mark_redraw();
    }

    void ListBox::updateControlStates() {
        if (maxPages() == 1) {
            _navLL->hide();
            _navL->hide();
            _navR->hide();
            _navRR->hide();
            _pageLabel->hide();
            return;
        }
        if (maxPages() > 2) {
            _navLL->show();
            _navRR->show();
        } else {
            _navLL->hide();
            _navRR->hide();
        }
        _navL->show();
        _navR->show();
        _pageLabel->show();
        if (currentPage() == 1) {
            _navLL->disable();
            _navL->disable();
            _navR->enable();
            _navRR->enable();
        } else if (currentPage() == maxPages()) {
            _navLL->enable();
            _navL->enable();
            _navR->disable();
            _navRR->disable();
        } else {
            _navLL->enable();
            _navL->enable();
            _navR->enable();
            _navRR->enable();
        }
        layout_buttons();
    }

    void ListBox::updatePageDisplay() {
        stringstream ss;
        ss << "[ " << currentPage() << '/' << maxPages() << " ]";
        _pageLabel->text = ss.str();
        _pageLabel->mark_redraw();
        updateControlStates();
    }

    void ListBox::LL_CLICK(void *v) {
        pageOffset = max(0, pageOffset - (maxPages() > 10 ? 10 : 5));
        updatePageDisplay();
        mark_redraw();
    }

    void ListBox::L_CLICK(void *v) {
        pageOffset--;
        updatePageDisplay();
        mark_redraw();
    }

    void ListBox::R_CLICK(void *v) {
        pageOffset++;
        updatePageDisplay();
        mark_redraw();
    }

    void ListBox::RR_CLICK(void *v) {
        pageOffset = min(pageOffset + (maxPages() > 10 ? 10 : 5), maxPages() - 1);
        updatePageDisplay();
        mark_redraw();
    }

    void ListBox::trim_texts() {
        for (const auto &it: contents) {
            auto wd = it->_widget;
            wd->text = utils::clip_string(it->label, wd->w, wd->h, ui::Widget::style.font_size);
            wd->mark_redraw();
        }
    }

//first, filter contents with our predicate and copy to current view
//second, sort current view
    void ListBox::refresh_list() {
        _sortedView.clear();
        for (auto &item: contents) {
            if (!filterPredicate || filterPredicate(item)) {
                _sortedView.push_back(item);
            }
        }

        if (sortPredicate)
            std::sort(_sortedView.begin(), _sortedView.end(), sortPredicate);
        else
            std::sort(_sortedView.begin(), _sortedView.end());

        auto offset = pageOffset * pageSize();
        auto count = std::min((int) pageSize(), (int) _sortedView.size() - offset);
        _currentView.clear();
        for (int i = offset; i < offset + count; i++) {
            _currentView.push_back(_sortedView[i]);
        }

        pageOffset = max(0, min(maxPages() - 1, pageOffset));

        updatePageDisplay();
    }

    void ListBox::selectIndex(int index) {
        if (index >= (int) _currentView.size()) {
            fprintf(stderr, "selectIndex out of bounds: idx[%d]\n", index);
            return;
        }
        auto item = _currentView[index];
        if (item->_selected) {
            selectedItems.erase(item);
            item->_selected = false;
            events.deselected(item);
        } else {
            item->_selected = true;
            if (multiSelect)
                selectedItems.emplace(item);
            else {
                for (const auto &si: selectedItems) {
                    si->_selected = false;
                    events.deselected(si);              // it is important to fire deselect events
                }                                       // |
                selectedItems.clear();                  // |
                selectedItems.emplace(item);            // |
            }                                           // V
            events.selected(item);                      // before firing select event
        }
    }
}