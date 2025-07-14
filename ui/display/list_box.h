//
// Created by brant on 2/5/24.
//

#pragma once

#include <unordered_set>
#include <utility>
#include "widgets.h"
#include "buttons/buttons.h"
#include "input/input.h"

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

            explicit ListItem(const string& label) : label(label), key(label) {};          //label
            explicit ListItem(const string& label, std::any  object) : label(label), object(std::move(object)), key(label) {};
            string label;                //the text displayed in the listbox. May only be one line.
            std::any object;             //an optional pointer to any data you want to keep a reference to
            string key;                  //optional sanitized string for sorting (defaults to value in label)
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
        std::function<bool(const shared_ptr<ListItem> &, const shared_ptr<ListItem> &)> sortPredicate;

        bool selectable = true; //allow selecting of entries at all
        bool multiSelect = true; //allow selecting more than one entry

        int pageSize();

        int currentPage() const;

        int maxPages();

        //please call mark_redraw() on this widget after editing contents or selections
        vector<shared_ptr<ListItem>> contents;
        std::unordered_set<shared_ptr<ListItem>> selectedItems;

        ListBox(int x, int y, int w, int h, int itemHeight, const shared_ptr<ui::InnerScene>& s);

        ListBox(int x, int y, int w, int h, int itemHeight, const vector<string>& items, ui::Scene& scene);

        shared_ptr<ListItem> add(const string& label, const std::any& object = nullptr);
        shared_ptr<ListItem> add(const string& label, const string& key, const std::any& object = nullptr);
        bool remove(const string& label);
        void removeAt(int index);
        void clear();
        void on_reflow() override;
        void undraw() override;
        void render() override;
        bool select(const string& label);
        //check the Y position relative to top of widget, divide by itemHeight
        void on_mouse_click(input::SynMotionEvent &ev) override;
        std::vector<shared_ptr<ListItem>> sortedItems() {return _sortedView; }
        //first, filter contents with our predicate and copy to current view
        //second, sort current view
        virtual void refresh_list();
    protected:

        std::vector<shared_ptr<ListItem>> _currentView;
        std::vector<shared_ptr<ListItem>> _sortedView;
        shared_ptr<ui::Text> _pageLabel;
        shared_ptr<ImageButton> _navLL, _navL, _navR, _navRR;
        void layout_buttons();
        void debugRender() override;

    private:
        //TODO: style sheets
        int itemHeight;
        int padding = 5;
        int pageOffset = 0;

        void selectIndex(int index);

        void updateControlStates();
        void updatePageDisplay();
        void LL_CLICK(void* v);
        void L_CLICK(void* v);
        void R_CLICK(void* v);
        void RR_CLICK(void* v);
        void trim_texts();

    };
}
