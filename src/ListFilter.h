//
// Created by brant on 7/14/25.
//

#pragma once

/*
 * manage the view of the package listbox
 * accept a ptr to package panel, filter panel, search box
 * maintain two internal lists from the opkg packages
 * one is the result of the text search
 * the other is the result of all filters applied to search
 */

#include "rempack/rempack_widgets.h"
class ListFilter {
public:
    ListFilter(widgets::ListBox *filterPanel, widgets::ListBox *packagePanel);
    void updateLists(const shared_ptr<widgets::FilterOptions>& options, const std::string& query = "");
private:
    widgets::ListBox *filterPanel;
    widgets::ListBox *packagePanel;
};
