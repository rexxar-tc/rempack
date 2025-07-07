//
// Created by brant on 7/3/25.
//

#include "filter_overlay.h"

namespace widgets{
    void FilterOverlay::show() {
        //this->scene->pinned = true;
        ui::MainLoop::show_overlay(this->scene);
    }

    void FilterOverlay::render() {
        render_inside_fill();
    }

    void FilterOverlay::mark_redraw() {
        for (const auto &c: children)
            c->mark_redraw();
        this->dirty = 1;
    }

    void FilterOverlay::upate_event() {
        events.updated(*options);
        mark_redraw();
    }

    ui::Scene FilterOverlay::make_overlay() {
        int padding = 20;
        auto s = ui::make_scene();
        s->add(this);
        mark_redraw();
        //auto v = new ui::VerticalLayout(x, y, 500, 800, s);
        auto dw = 500 - padding - padding;
        auto dh = 800 - padding - padding;
        auto dx = x + padding;
        auto dy = y + padding;
        auto iTog = make_shared<ui::ToggleButton>(dx, dy, dw, 50, "Installed");
        iTog->toggled = options->Installed;
        iTog->style.justify = ui::Style::JUSTIFY::LEFT;
        iTog->events.toggled += [this](bool s) {
            options->Installed = s;
            upate_event();
        };
        s->add(iTog);
        children.push_back(iTog);
        dy += padding + iTog->h;
        auto uTog = make_shared<ui::ToggleButton>(dx, dy, dw, 50, "Only Upgradable");
        uTog->toggled = options->Upgradable;
        uTog->style.justify = ui::Style::JUSTIFY::LEFT;
        uTog->events.toggled += [this](bool s) {
            options->Upgradable = s;
            upate_event();
        };
        children.push_back(uTog);
        s->add(uTog);
        dy += padding + uTog->h;
        auto unTog = make_shared<ui::ToggleButton>(dx, dy, dw, 50, "Not Installed");
        unTog->toggled = options->NotInstalled;
        unTog->style.justify = ui::Style::JUSTIFY::LEFT;
        unTog->events.toggled += [this](bool s) {
            options->NotInstalled = s;
            upate_event();
        };
        s->add(unTog);
        children.push_back(unTog);
        dy += padding + unTog->h;
        auto descTog = make_shared<ui::ToggleButton>(dx, dy, dw, 50, "Search Descriptions");
        descTog->toggled = options->NotInstalled;
        descTog->style.justify = ui::Style::JUSTIFY::LEFT;
        descTog->events.toggled += [this](bool s) {
            options->SearchDescription = s;
            upate_event();
        };
        children.push_back(descTog);
        s->add(descTog);
        dy += padding + descTog->h;
        auto groupTog = make_shared<ui::ToggleButton>(dx, dy, dw, 50, "Group Splashscreens");
        groupTog->toggled = options->groupSplash;
        groupTog->style.justify = ui::Style::JUSTIFY::LEFT;
        groupTog->events.toggled += [this](bool s){
            options->groupSplash = s;
            upate_event();
        };
        children.push_back(groupTog);
        s->add(groupTog);
        dy += padding + groupTog->h;
        if (!options->Repos.empty()) {
            //TODO: set height of the list based on number of entries
            _repoList = make_shared<ListBox>(dx, dy, dw, 200, 25, s);
            for (auto &[r, set]: options->Repos) {
                auto item = _repoList->add(r);
                if (set) {
                    item->_selected = true;
                    _repoList->selectedItems.emplace(item);
                }
            }
            std::sort(_repoList->contents.begin(), _repoList->contents.end());
            _repoList->mark_redraw();
            _repoList->events.selected += [this](const shared_ptr<ListBox::ListItem> &li) {
                options->Repos[li->label] = true;
                upate_event();
            };
            _repoList->events.deselected += [this](const shared_ptr<ListBox::ListItem> &li) {
                options->Repos[li->label] = false;
                upate_event();
            };
            children.push_back(_repoList);
            s->add(_repoList);
            dy += padding + _repoList->h;
        }
        if (!options->Licenses.empty()) {
            _licenseList = make_shared<ListBox>(dx, dy, dw, 200, 25, scene);
            for (auto &[l, set]: options->Licenses) {
                auto item = _licenseList->add(l);
                if (set) {
                    item->_selected = true;
                    _licenseList->selectedItems.emplace(item);
                }
            }
            children.push_back(_licenseList);
            s->add(_licenseList);
            _licenseList->mark_redraw();
            _licenseList->events.selected += [this](const shared_ptr<ListBox::ListItem> &li) {
                options->Licenses[li->label] = true;
                upate_event();
            };
            _licenseList->events.deselected += [this](const shared_ptr<ListBox::ListItem> &li) {
                options->Licenses[li->label] = false;
                upate_event();
            };
        }
        return s;
    }}