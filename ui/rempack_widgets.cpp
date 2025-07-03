//
// Created by brant on 7/2/25.
//

#include "include/rempack_widgets.h"
#include "opkg.h"

namespace widgets {
    void TerminalDialog::build_dialog() {
        this->create_scene();
        layout = make_shared<ui::VerticalLayout>(x, y, w, h, this->scene);
        int dx = padding;
        int dy = padding;
        int dw = w - padding - padding;
        t1 = new ui::Text(dx, dy, dw, utils::line_height(), title);
        int dht = h - padding - padding - 50 - utils::line_height();
        buffer_size = dht / (utils::line_height() + padding);
        layout->pack_start(t1);
        l1 = new ui::MultiText(dx, dy + padding, dw, dht, "Running...");
        layout->pack_start(l1);
        auto button_bar = new ui::HorizontalLayout(0, 0, this->w, 50, this->scene);
        layout->pack_end(button_bar);
        button_bar->y -= 2;

        // layout->reflow();

        this->add_buttons(button_bar);
        ui::TaskQueue::add_task([this]() { this->update_texts(); });
    }

    void TerminalDialog::on_reflow() {
        l1->on_reflow();
    }

    void TerminalDialog::mark_redraw() {
//layout->refresh();
        Overlay::mark_redraw();
    }

    void TerminalDialog::stdout_callback(const std::string &s) {
        std::cout << s;
        auto default_fs = ui::Style::DEFAULT.font_size;
        auto lines = utils::wrap_string(s, w - padding - padding, default_fs);
        for (const auto &l: lines)
            push_line(l);
        update_texts();
        //mark_redraw();
    }

    void TerminalDialog::set_callback(const std::function<void()> &cb) {
        callback = cb;
    }

    void TerminalDialog::on_button_selected(std::string s) {
        callback();
        widgets::Overlay::on_button_selected(s);
    }

    void TerminalDialog::add_buttons_reflow(ui::HorizontalReflow *button_bar) {
        auto default_fs = ui::Style::DEFAULT.font_size;
        for (auto b: this->buttons) {
            auto image = stbtext::get_text_size(b, default_fs);

            button_bar->pack_start(new ui::DialogButton(20, 0, image.w + default_fs, 50, this, b));
        }
    }

    void TerminalDialog::push_line(const std::string &l) {
        consoleBuffer.push_back(l);
        if (consoleBuffer.size() > buffer_size)
            consoleBuffer.pop_front();
    }


    void TerminalDialog::update_texts() {
        if (!t1 || !l1)
            return;


        std::stringstream ss;
        for (const auto &l: consoleBuffer)
            ss << l << std::endl;
        auto str = ss.str();
        l1->set_text(str.substr(0, str.size() - 2));
        l1->undraw();
        l1->mark_redraw();
        on_reflow();
        //Overlay::mark_redraw();
    }


    void SearchBox::on_reflow() {
        pixmap->set_coords(x + w - h, y, h, h);
        pixmap->mark_redraw();
        RoundedTextInput::on_reflow();
    }

    void SearchBox::on_mouse_click(input::SynMotionEvent &ev) {
        ev.stop_propagation();
        _keyboard->show();
    }

    void SearchBox::onChange(KeyboardEvent ev) {
        //need to do a full undraw in case characters were removed
        RoundedTextInput::undraw();
        set_text(_keyboard->text + '_');
        //redraw the button we just erased
        pixmap->mark_redraw();
    }

    void SearchBox::onDone(KeyboardEvent ev) {
        set_text(ev.text);
        on_done(ev.text);
    }

    void MenuOverlay::show() {
        //this->scene->pinned = true;
        ui::MainLoop::show_overlay(this->scene, true);
        ui::MainLoop::refresh();
    }

    void MenuOverlay::hide() {
        ui::MainLoop::hide_overlay(this->scene);
        ui::MainLoop::refresh();
    }

    void MenuOverlay::render() {
        render_inside_fill();
    }

    void MenuOverlay::mark_redraw() {
        for (const auto &c: children)
            c->mark_redraw();
        this->dirty = 1;
    }

    void MenuOverlay::upate_event() {
        events.updated(data);
        mark_redraw();
    }

    void MenuOverlay::refresh_event(void *) {
        auto td = new TerminalDialog(500, 500, 800, 1100, "opkg update");
        td->set_callback([this]() { this->hide(); });
        td->show();
        ui::TaskQueue::add_task([=]() {
            auto ret = opkg::UpdateRepos([=](const string &s) { td->stdout_callback(s); });
            if (ret == 0)
                td->stdout_callback("Done.");
            else
                td->stdout_callback("Error!");
            std::cout << "opkg update returned with exit code " << ret << std::endl;
        });
    }

    ui::Scene MenuOverlay::make_overlay() {
        int padding = 20;
        auto s = ui::make_scene();
        s->add(this);
        mark_redraw();
        auto v = ui::VerticalLayout(x, y + padding, 500, 800 - (padding * 4), s);
        auto dw = 500 - padding - padding;
        auto dh = 800 - padding - padding;
        auto dx = v.x + padding;
        auto dy = v.y + padding;
        auto updateBtn = new EventButton(padding, padding, dw, utils::line_height(), "Refresh Repositories");
        updateBtn->events.clicked += PLS_DELEGATE(refresh_event);
        v.pack_start(updateBtn);
        auto exitBtn = new EventButton(padding, 0 - padding - utils::line_height(), dw, utils::line_height(), "Quit");
        exitBtn->events.clicked += PLS_LAMBDA(auto &ev) { exit(0); };
        v.pack_end(exitBtn);
        return s;
    }

    void ConfigButton::on_overlay_hidden(ui::InnerScene::DialogVisible v) {
        //events.updated(options);
    }

    void ConfigButton::on_mouse_click(input::SynMotionEvent &ev) {
        ev.stop_propagation();

        auto ov = new MenuOverlay(x - 500, y + h, 500, 800, data);
        ov->scene->on_hide += PLS_DELEGATE(on_overlay_hidden);
//ov->events.updated += [this, &ov](FilterOptions* o){events.updated(options);};
        ov->show();
        ui::MainLoop::refresh();
    }

    shared_ptr<ListBox> _licenseList;

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
    }

    void FilterButton::on_overlay_hidden(ui::InnerScene::DialogVisible v) {
        //events.updated(options);
    }

    void FilterButton::on_mouse_click(input::SynMotionEvent &ev) {
        ev.stop_propagation();

        auto ov = new FilterOverlay(x + w, y + h, 500, 800, options);
        ov->scene->on_hide += [=](auto &d) { on_overlay_hidden(d); };
        ov->events.updated += [=](FilterOptions &o) { events.updated(o); };
        ov->show();
        ui::MainLoop::refresh();
    }


    void PackageInfoPanel::on_reflow() {
        _text->set_coords(x + padding, y + padding, w - (2 * padding), h - (2 * padding) - controlHeight);
        _text->mark_redraw();
        layout_buttons();
    }

    void PackageInfoPanel::set_text(string text) {
        _text->undraw();
        _text->text = std::move(text);
        _text->mark_redraw();
        this->mark_redraw();
    }

    void PackageInfoPanel::set_states(bool installed, bool canPreview, bool showDownload) {
        if (installed) {
            _installBtn->disable();
            _removeBtn->enable();
        } else {
            _installBtn->enable();
            _removeBtn->disable();
        }
        if (canPreview)
            _previewBtn->show();
        else
            _previewBtn->hide();
        if (showDownload)
            _downloadBtn->show();
        else
            _downloadBtn->hide();
    }

    void PackageInfoPanel::set_actions(int add, int remove) {
        stringstream ss;
        ss << "Pending: [+" << add << "/-" << remove << "]";
        _actionCounter->textWidget->text = ss.str();
        _actionCounter->text = ss.str();
        _actionCounter->mark_redraw();
        _actionCounter->textWidget->mark_redraw();
        mark_redraw();
    }

    void PackageInfoPanel::layout_buttons() {
        auto dx = x + padding;
        auto dy = y + h - padding - controlHeight;
        _installBtn->set_coords(dx, dy, controlWidth, controlHeight);
        dx += controlWidth + padding;
        _removeBtn->set_coords(dx, dy, controlWidth, controlHeight);
        dx += controlWidth + padding;
        _downloadBtn->set_coords(dx, dy, controlWidth, controlHeight);
        dx += controlWidth + padding;
        _previewBtn->set_coords(dx, dy, controlWidth, controlHeight);

        _actionCounter->set_coords(w - controlWidth - padding, dy, controlWidth, controlHeight);

        _installBtn->on_reflow();
        _removeBtn->on_reflow();
        _downloadBtn->on_reflow();
        _previewBtn->on_reflow();
        _actionCounter->on_reflow();
        _installBtn->mark_redraw();
        _removeBtn->mark_redraw();
        _downloadBtn->mark_redraw();
        _previewBtn->mark_redraw();
        _actionCounter->mark_redraw();
    }

    void InstallDialog::build_dialog() {
        this->create_scene();
        int padding = 20;
        unordered_map<string, uint> items;
        for (const auto &pk: packages) {
            widget_helpers::format_deps_recursive(items, pk);
        }
        stringstream s1;
        s1 << "Installing " << packages.size() << " packages and ";
        s1 << items.size() - packages.size() << " dependencies:";
        uint totalSize = 0;
        vector<string> labels;
        for (const auto &[n, s]: items) {
            labels.push_back(n);
            totalSize += s;
        }
        stringstream s2;
        s2 << "This will require approximately " << utils::stringifySize(totalSize);

        auto layout = ui::VerticalLayout(x, y, w, h, this->scene);
        int dx = padding;
        int dy = padding;
        int dw = w - padding - padding;
        auto t1 = new ui::Text(dx, dy, dw, utils::line_height(), s1.str());
        layout.pack_start(t1);
        int lh = min((int) 300, (int) ((labels.size()) * (utils::line_height() + 5)) + 10);
        auto l1 = new widgets::ListBox(dx, dy + padding, dw, lh, utils::line_height(), scene);
        for (const auto &line: labels) {
            l1->add(line);
        }
        l1->selectable = false;
        layout.pack_start(l1);
        auto t2 = new ui::Text(dx, dy + padding + padding, dw, utils::line_height(), s2.str());
        layout.pack_start(t2);
        auto button_bar = ui::HorizontalLayout(0, 0, this->w, 50, this->scene);
        layout.pack_end(button_bar);
        button_bar.y -= 2;

        this->add_buttons(&button_bar);
    }

    void InstallDialog::setCallback(const function<void(bool)> &callback) {
        _callback = callback;
    }

    void InstallDialog::on_button_selected(std::string s) {
        if (s == "OK") {
            _accepted = true;
            run_install();
            return;
        } else {
            _accepted = false;
        }
        widgets::Overlay::on_button_selected(s);
        if (_callback)
            _callback(_accepted);
    }

    void InstallDialog::run_install() {
        auto td = new TerminalDialog(500, 500, 800, 1100, "opkg install");
        td->set_callback([this]() {
            if (this->_callback)
                this->_callback(this->_accepted);
            this->hide();
        });
        td->show();
        ui::TaskQueue::add_task([=]() {
            auto ret = opkg::Install(packages, [=](const string s) { td->stdout_callback(s); });
            if (ret == 0)
                td->stdout_callback("Done.");
            else
                td->stdout_callback("Error!");
            std::cout << "opkg install returned with exit code " << ret << std::endl;
        });
    }

    void UninstallDialog::build_dialog() {
        this->create_scene();
        layout = make_shared<ui::VerticalLayout>(x, y, w, h, this->scene);
        int dx = padding;
        int dy = padding;
        int dw = w - padding - padding;
        t1 = new ui::Text(dx, dy, dw, utils::line_height(), "Loading...");
        layout->pack_start(t1);
        l1 = new widgets::ListBox(dx, dy + padding, dw, utils::line_height(), utils::line_height(), scene);
        l1->selectable = false;
        layout->pack_start(l1);
        cb = new ui::ToggleButton(dx, dy + padding + padding, dw, utils::line_height(), "Auto-remove dependencies");
        cb->toggled = dependencies;
        cb->events.toggled += [this](bool s) { on_autoremove_tick(s); };
        cb->style.justify = ui::Style::JUSTIFY::LEFT;
        layout->pack_start(cb);
        t2 = new ui::Text(dx, dy + padding + padding, dw, utils::line_height(), "");
        layout->pack_start(t2);
        auto button_bar = new ui::HorizontalLayout(0, 0, this->w, 50, this->scene);
        layout->pack_end(button_bar);
        button_bar->y -= 2;

// layout->reflow();

        this->add_buttons(button_bar);
        ui::TaskQueue::add_task([this]() { this->update_texts(); });
    }

    void UninstallDialog::on_reflow() {
        l1->on_reflow();
    }

    void UninstallDialog::mark_redraw() {
        layout->refresh();
    }

    void UninstallDialog::setCallback(const function<void(bool)> &callback) {
        _callback = callback;
    }

    void UninstallDialog::on_button_selected(std::string s) {
        if (s == "OK") {
            _accepted = true;
            run_uninstall();
            return;
        } else
            _accepted = false;
        widgets::Overlay::on_button_selected(s);
        if (_callback)
            _callback(_accepted);
    }

    void UninstallDialog::on_autoremove_tick(bool state) {
        dependencies = state;
        ui::TaskQueue::add_task([this]() { this->update_texts(); });
    }

    void UninstallDialog::add_buttons_reflow(ui::HorizontalReflow *button_bar) {
        auto default_fs = ui::Style::DEFAULT.font_size;
        for (auto b: this->buttons) {
            auto image = stbtext::get_text_size(b, default_fs);

            button_bar->pack_start(new ui::DialogButton(20, 0, image.w + default_fs, 50, this, b));
        }
    }

    void UninstallDialog::run_uninstall() {
        auto td = new TerminalDialog(500, 500, 800, 1100, "opkg uninstall");
        td->set_callback([this]() {
            if (this->_callback)
                this->_callback(this->_accepted);
            this->hide();
        });
        td->show();
        auto ret = opkg::Uninstall(packages, [td](const string s) { td->stdout_callback(s); },
                                   dependencies ? " --autoremove" : "");
        if (ret == 0) {
            td->stdout_callback("Done.");
            for (const auto &t: packages)
                t->State = package::NotInstalled;
        } else {
            td->stdout_callback("Error!");
        }
        std::cout << "opkg uninstall returned with exit code " << ret << std::endl;
    }

    void UninstallDialog::update_texts() {
        if (!t1 || !t2 || !l1)
            return;
        results.clear();
        auto ret = opkg::Instance->ComputeUninstall(packages, dependencies, &results);
        if (ret != 0) {
            printf("OPKG ERROR! %d\n", ret);
        }
        stringstream s1;
        s1 << "Uninstalling " << packages.size() << " packages and ";
        s1 << results.size() - packages.size() << " dependencies:";

        t1->set_text(s1.str());

        uint totalSize = 0;
        int lh = min((int) 300, (int) ((results.size()) * (utils::line_height() + 5)) + 10);
        l1->h = lh;
        l1->clear();
        for (const auto &pk: results) {
            l1->add(pk->Package);
            totalSize += pk->Size;
        }
        cb->y = lh + padding + l1->y;
        cb->mark_redraw();
        stringstream s2;
        s2 << "This will free approximately " << utils::stringifySize(totalSize);
        t2->y = cb->y + padding + cb->h;
        t2->set_text(s2.str());
        on_reflow();
        //Overlay::mark_redraw();
    }
}