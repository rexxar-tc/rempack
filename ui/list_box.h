//
// Created by brant on 2/5/24.
//

#pragma once

#include <unordered_set>
#include "widgets.h"
#include "buttons.h"

namespace widgets {
    /*
     * Displays a list of items.
     * Provides sorting and filtering delegates to control displayed items, as well as
     * text clipping and pagination.
     * Features multiselect, uniselect, read only modes. Fires events when de/selected
     * and when items added/removed from contents.
     * Items are represented as a simple string with an attached std::any object which can store any user data.
     * Reflow is supported: list will be reformatted when the control is resized or moved.
     *
     * Presently only single-line items are supported; overflowing text will be trimmed.
     * Each item in the list must have a unique label.
     *
     * ___________________________
     * | Item 1                  |
     * | Item 2                  |
     * | Item 3 with a clipped la|
     * | [1/4] [<<] [<] [>] [>>] |
     * ---------------------------
     * [page number] [back 5/10] [back 1] [forward 1] [forward 5/10]
     * Fast nav buttons switch from 5 to 10 pages when there are more than 10 pages.
     * Keypad to enter page number would be nice
     */
    class ListBox : public RoundCornerWidget {
    public:
        struct ListItem {
            friend class ListBox;

            explicit ListItem(string label) : label(std::move(label)) {};          //label
            explicit ListItem(string label, std::any object) : label(std::move(label)), object(std::move(object)) {};
            string label;                //the text displayed in the listbox. May only be one line.
            std::any object;             //an optional pointer to any data you want to keep a reference to

            //all items must be unique. This sucks, but we'll implement it if someone needs it
            inline bool operator==(const ListItem &other) const {
                return this->label == other.label;
            }

            bool _selected = false;
        private:
            shared_ptr<ui::Text> _widget = nullptr;
        };

        PLS_DEFINE_SIGNAL(LISTBOX_EVENT, const shared_ptr<ListItem>);

        class LISTBOX_EVENTS {
        public:
            LISTBOX_EVENT selected;
            LISTBOX_EVENT deselected;
            LISTBOX_EVENT added;
            LISTBOX_EVENT removed;
        };

        LISTBOX_EVENTS events;

        std::function<bool(const shared_ptr<ListItem> &)> filterPredicate;
        std::function<bool(const shared_ptr<ListItem> &, shared_ptr<ListItem> &)> sortPredicate;

        bool selectable = true; //allow selecting of entries at all
        bool multiSelect = true; //allow selecting more than one entry

        int pageSize() {
            auto size = (int)floor(float(h - padding - padding) / float(itemHeight + padding));
            if(size < (int)_sortedView.size())   //if we have more items than will fit on one page,
                size--;                          //reserve at least one line of space at the bottom of the view for the nav elements
            return size;
        }

        int currentPage() const {
            return pageOffset + 1;
        }

        int maxPages(){
            return (int)ceil((float)_sortedView.size() / (float)pageSize());
        }

        //please call mark_redraw() on this widget after editing contents or selections
        vector<shared_ptr<ListItem>> contents;
        std::unordered_set<shared_ptr<ListItem>> selectedItems;

        ListBox(int x, int y, int w, int h, int itemHeight, const shared_ptr<ui::InnerScene>& s) : RoundCornerWidget(x, y, w, h, RoundCornerStyle()) {
            this->itemHeight = itemHeight;
            _pageLabel = make_shared<ui::Text>(0,0,w,itemHeight,"");

            _navLL = make_shared<ImageButton>(0,0,itemHeight,itemHeight,ICON(assets::png_fast_arrow_left_png));
            _navL = make_shared<ImageButton>(0,0,itemHeight,itemHeight,ICON(assets::png_nav_arrow_left_png));
            _navR = make_shared<ImageButton>(0,0,itemHeight,itemHeight,ICON(assets::png_nav_arrow_right_png));
            _navRR = make_shared<ImageButton>(0,0,itemHeight,itemHeight,ICON(assets::png_fast_arrow_right_png));
            _navLL->hide();
            _navL->hide();
            _navR->hide();
            _navRR->hide();
            _pageLabel->hide();
            children.push_back(_navLL);
            children.push_back(_navL);
            children.push_back(_navR);
            children.push_back(_navRR);
            children.push_back(_pageLabel);
            s->add(_navLL);
            s->add(_navL);
            s->add(_navR);
            s->add(_navRR);
            s->add(_pageLabel);

            _navLL->events.clicked += PLS_DELEGATE(LL_CLICK);
            _navL->events.clicked += PLS_DELEGATE(L_CLICK);
            _navR->events.clicked += PLS_DELEGATE(R_CLICK);
            _navRR->events.clicked += PLS_DELEGATE(RR_CLICK);
            layout_buttons();
        }

        ListBox(int x, int y, int w, int h, int itemHeight, const vector<string>& items, ui::Scene& scene): ListBox(x,y,w,h,itemHeight,scene) {
            for(const auto &s: items){
                this->add(s);
            }
        }

        shared_ptr<ListItem> add(const string& label, const std::any& object = nullptr) {
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

        bool remove(const string& label) {
            int i = 0;
            shared_ptr<ListItem> item = nullptr;
            for (; i < (int)contents.size(); i++) {
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

        void removeAt(int index) {
            auto item = contents[index];
            auto w = item->_widget;
            contents.erase(contents.begin() + index);
            events.removed(item);
            mark_redraw();
        }

        void clear(){
            contents.clear();
            mark_redraw();
        }

        void on_reflow() override {
            layout_buttons();
            trim_texts();
            mark_redraw();
        }

        void undraw() override {
            //fb->draw_rect(this->x, this->y, this->w, this->h, WHITE, true);
            RoundCornerWidget::render_inside_fill();
            RoundCornerWidget::undraw();
        }

        void render() override {
            undraw();
            refresh_list();
            int sx = this->x + padding;
            int sy = this->y + padding;
            for (auto item: _currentView) {
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
        void on_mouse_click(input::SynMotionEvent &ev) override {
            //ev.stop_propagation();
            if(!selectable)
                return;
            auto hgt = itemHeight + padding;
            auto sy = ev.y - this->y;
            auto shgt = sy / hgt;
            int idx = floor(shgt);
            //printf("Click at %d,%d: computed offset %d: displayed %d\n", ev.x, ev.y, idx, displayed_items());
            if (idx >= (int)_currentView.size())
                return;
            selectIndex(idx);
            mark_redraw();
        }

    protected:
        std::vector<shared_ptr<ListItem>> _currentView;
        std::vector<shared_ptr<ListItem>> _sortedView;
        shared_ptr<ui::Text> _pageLabel;
        shared_ptr<ImageButton> _navLL, _navL, _navR, _navRR;

        void layout_buttons() {
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
    private:
        //TODO: style sheets
        int itemHeight;
        int padding = 5;

        int pageOffset = 0;

        void updateControlStates(){
            if(maxPages() == 1){
                _navLL->hide();
                _navL->hide();
                _navR->hide();
                _navRR->hide();
                _pageLabel->hide();
                return;
            }
            if(maxPages() > 2) {
                _navLL->show();
                _navRR->show();
            }
            else{
                _navLL->hide();
                _navRR->hide();
            }
            _navL->show();
            _navR->show();
            _pageLabel->show();
            if(currentPage() == 1){
                _navLL->disable();
                _navL->disable();
                _navR->enable();
                _navRR->enable();
            }
            else if (currentPage() == maxPages()){
                _navLL->enable();
                _navL->enable();
                _navR->disable();
                _navRR->disable();
            }
            else{
                _navLL->enable();
                _navL->enable();
                _navR->enable();
                _navRR->enable();
            }
            layout_buttons();
        }

        void updatePageDisplay(){
            stringstream ss;
            ss << "[ " << currentPage() << '/' << maxPages() << " ]";
            _pageLabel->text = ss.str();
            _pageLabel->mark_redraw();
            updateControlStates();
        }

        void LL_CLICK(void* v){
            pageOffset = max(0, pageOffset - (maxPages() > 10 ? 10 : 5));
            updatePageDisplay();
            mark_redraw();
        }

        void L_CLICK(void* v) {
            pageOffset--;
            updatePageDisplay();
            mark_redraw();
        }
        void R_CLICK(void* v){
            pageOffset++;
            updatePageDisplay();
            mark_redraw();
        }
        void RR_CLICK(void* v){
            pageOffset = min(pageOffset + (maxPages() > 10 ? 10 : 5), maxPages() - 1);
            updatePageDisplay();
            mark_redraw();
        }

        void trim_texts() {
            for (const auto &it: contents) {
                auto wd = it->_widget;
                wd->text = utils::clip_string(it->label, wd->w, wd->h, ui::Widget::style.font_size);
                wd->mark_redraw();
            }
        }

        //first, filter contents with our predicate and copy to current view
        //second, sort current view
        virtual void refresh_list() {
            _sortedView.clear();
            for (auto &item: contents) {
                if (!filterPredicate || filterPredicate(item)) {
                    _sortedView.push_back(item);
                }
            }

            if(sortPredicate)
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

        void selectIndex(int index) {
            if (index >= (int)_currentView.size()) {
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

    };
}
