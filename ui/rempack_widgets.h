//
// Created by brant on 2/5/24.
//

#pragma once

#include "widgets.h"
#include "list_box.h"
#include "keyboard.h"
#include "overlay.h"
#include <utility>
#include "../opkg/opkg.h"
#include "../src/widget_helpers.h"

namespace widgets{

    /*
    * _________________________________________________
    * | Window Title                                  |
    * | _____________________________________________ |
    * | | Scrolling                                 | |
    * | | Text                                      | |
    * | | Contents                                  | |
    * | ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯ |
    * | [Close]                                       |
    * ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    */
    class TerminalDialog: public widgets::Overlay {
    public:
        TerminalDialog(int x, int y, int w, int h, const std::string& title): Overlay(x,y,w,h){
            this->buttons.clear();
            this->buttons.emplace_back("Dismiss");
        }
        void build_dialog() override {
            this->create_scene();
            layout = make_shared<ui::VerticalLayout>(x, y, w, h, this->scene);
            int dx = padding;
            int dy = padding;
            int dw = w - padding - padding;
            t1 = new ui::Text(dx, dy, dw, utils::line_height(), title);
            int dht = h - padding - padding - 50 - utils::line_height();
            buffer_size = dht / (utils::line_height() + padding);
            layout->pack_start(t1);
            l1 = new ui::MultiText(dx, dy + padding, dw,  dht, "Running...");
            layout->pack_start(l1);
            auto button_bar = new ui::HorizontalLayout(0, 0, this->w, 50, this->scene);
            layout->pack_end(button_bar);
            button_bar->y -= 2;

            // layout->reflow();

            this->add_buttons(button_bar);
            ui::TaskQueue::add_task([this](){this->update_texts();});
        }
        void on_reflow() override{
            t1->on_reflow();
        }
        void mark_redraw() override{
            //layout->refresh();
            Overlay::mark_redraw();
        }
        void stdout_callback(const std::string &s){
            auto default_fs = ui::Style::DEFAULT.font_size;
            auto lines = utils::wrap_string(s, w - padding - padding, default_fs);
            for(const auto &l : lines)
                push_line(l);
            update_texts();
            //mark_redraw();
        }
        void set_callback(const std::function<void()> &cb){
            callback = cb;
        }
    private:
        const int padding = 20;
        shared_ptr<ui::VerticalLayout> layout;
        ui::Text *t1 = nullptr;
        ui::MultiText *l1 = nullptr;
        std::deque<std::string> consoleBuffer;
        int buffer_size = 16;
        std::function<void()> callback;

        void on_button_selected(std::string s) override{
            callback();
            widgets::Overlay::on_button_selected(s);
        }

        void add_buttons_reflow(ui::HorizontalReflow *button_bar) {
            auto default_fs = ui::Style::DEFAULT.font_size;
            for (auto b: this->buttons) {
                auto image = stbtext::get_text_size(b, default_fs);

                button_bar->pack_start(new ui::DialogButton(20, 0, image.w + default_fs, 50, this, b));
            }
        }

        void push_line(const std::string &l){
            consoleBuffer.push_back(l);
            if(consoleBuffer.size() > buffer_size)
                consoleBuffer.pop_front();
        }


        void update_texts(){
            if(!t1 || !l1)
                return;


            std::stringstream ss;
            for(const auto &l : consoleBuffer)
                ss << l << std::endl;
            auto str = ss.str();
            l1->set_text(str.substr(0, str.size() - 2));
            l1->mark_redraw();
            //on_reflow();
            //Overlay::mark_redraw();
        }
    };

    class SearchBox : public RoundedTextInput {
    public:
        SearchBox(int x, int y, int w, int h, RoundCornerStyle style, const string text = "") : RoundedTextInput(x, y, w, h, style, text) {
            //TODO: style sheets
            pixmap = make_shared<ui::Pixmap>(x + w - h, y, h, h, ICON(assets::png_search_png));
            children.push_back(pixmap);
            _keyboard = new Keyboard();
            _keyboard->events.changed += PLS_DELEGATE(onChange);
            _keyboard->events.done += PLS_DELEGATE(onDone);

        }

        void on_reflow() override{
            pixmap->set_coords(x + w - h, y, h, h);
            pixmap->mark_redraw();
            RoundedTextInput::on_reflow();
        }

        void on_mouse_click(input::SynMotionEvent &ev) override{
            ev.stop_propagation();
            _keyboard->show();
        }
    private:
        widgets::Keyboard *_keyboard;
        shared_ptr<ui::Pixmap> pixmap;

        void onChange(KeyboardEvent ev){
            //need to do a full undraw in case characters were removed
            RoundedTextInput::undraw();
            set_text(_keyboard->text + '_');
            //redraw the button we just erased
            pixmap->mark_redraw();
        }
        void onDone(KeyboardEvent ev){
            set_text(ev.text);
            on_done(ev.text);
        }
    };

    struct MenuData{
        unordered_set<string> PendingInstall;
        unordered_set<string> PendingRemove;
        bool Cronch;
        int FontSize;
    };


    class MenuOverlay: RoundCornerWidget{
    public:
        ui::Scene scene;
        MenuData *data;
        MenuOverlay(int x, int y, int w, int h, MenuData *currentData): RoundCornerWidget(x,y,w,h,RoundCornerStyle()) {
            data = currentData;
            scene = make_overlay();
        }

        void show() {
            //this->scene->pinned = true;
            ui::MainLoop::show_overlay(this->scene, true);
            ui::MainLoop::refresh();
        }

        void render() override{
            render_inside_fill();
        }

        void mark_redraw() override{
            for(const auto &c: children)
                c->mark_redraw();
            this->dirty=1;
        }

        PLS_DEFINE_SIGNAL(OMENU_EVENT, MenuData*);

        class OMENU_EVENTS {
        public:
            OMENU_EVENT updated;
        };

        OMENU_EVENTS events;

    private:
        void upate_event(){
            events.updated(data);
            mark_redraw();
        }
        void refresh_event(void*) {
            auto td = new TerminalDialog(500,500,800,1100, "opkg update");
            td->set_callback([this](){this->hide();});
            td->show();
            ui::TaskQueue::add_task([=](){
                auto ret = opkg::UpdateRepos([=](const string s){td->stdout_callback(s);});
                if (ret == 0)
                    td->stdout_callback("Done.");
                else
                    td->stdout_callback("Error!");
                std::cout << "opkg update returned with exit code " << ret << std::endl;
            });
        }
        /*
         * [x] Check upgrades
         * UI Style:
         * (*) Cosy
         * ( ) Compact
         * Font Size [10[^v]]
         */
        ui::Scene make_overlay() {
            int padding = 20;
            auto s = ui::make_scene();
            s->add(this);
            mark_redraw();
            auto v = ui::VerticalLayout(x, y + padding, 500, 800 - (padding * 4), s);
            auto dw = 500 - padding - padding;
            auto dh = 800 - padding - padding;
            auto dx = v.x + padding;
            auto dy = v.y + padding;
            auto updateBtn = new EventButton(padding,padding,dw,utils::line_height(),"Refresh Repositories");
            updateBtn->events.clicked += PLS_DELEGATE(refresh_event);
            v.pack_start(updateBtn);
            auto exitBtn = new EventButton(padding, 0-padding-utils::line_height(), dw, utils::line_height(), "Quit");
            exitBtn->events.clicked += PLS_LAMBDA(auto &ev) { exit(0); };
            v.pack_end(exitBtn);
            return s;
        }
    };

    class ConfigButton : public RoundImageButton {
    public:
        RoundCornerStyle style;
            MenuData *data;
        ConfigButton(int x, int y, int w, int h, MenuData *data, RoundCornerStyle style = RoundCornerStyle()) : RoundImageButton(x, y, w, h, ICON(assets::png_menu_png), style) {
            this->data = data;
        }
        void on_overlay_hidden(ui::InnerScene::DialogVisible v){
            //events.updated(options);
        }

        void on_mouse_click(input::SynMotionEvent &ev) override {
            ev.stop_propagation();

            auto ov = new MenuOverlay(x - 500, y + h, 500, 800, data);
            ov->scene->on_hide += PLS_DELEGATE(on_overlay_hidden);
            //ov->events.updated += [this, &ov](FilterOptions* o){events.updated(options);};
            ov->show();
            ui::MainLoop::refresh();
        }
    };

    struct FilterOptions{
        bool Installed;
        bool Upgradable;
        bool NotInstalled;
        bool SearchDescription;
        std::map<std::string, bool> Repos;
        std::map<std::string, bool> Licenses;
    };

    class FilterOverlay: RoundCornerWidget{
    public:
        ui::Scene scene;
        shared_ptr<FilterOptions> options;
        FilterOverlay(int x, int y, int w, int h, shared_ptr<FilterOptions> currentOptions): RoundCornerWidget(x,y,w,h,RoundCornerStyle()) {
            options = currentOptions;
            scene = make_overlay();
        }

        void show() {
            //this->scene->pinned = true;
            ui::MainLoop::show_overlay(this->scene);
        }

        void render() override{
            render_inside_fill();
        }

        void mark_redraw() override{
            for(const auto &c: children)
                c->mark_redraw();
            this->dirty=1;
        }

        PLS_DEFINE_SIGNAL(OFILTER_EVENT, FilterOptions);

        class OFILTER_EVENTS {
        public:
            OFILTER_EVENT updated;
        };

        OFILTER_EVENTS events;

    private:
        void upate_event(){
            events.updated(*options);
            mark_redraw();
        }
        /*
         * [ ] Installed
         * [ ] Upgradable
         *
         * __________________
         * | Available      |
         * | Repository list|
         * ------------------
         *
         */
        shared_ptr<ListBox> _repoList;
        shared_ptr<ListBox> _licenseList;
        ui::Scene make_overlay() {
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
            iTog->events.toggled += [this](bool s){options->Installed = s; upate_event();};
            s->add(iTog);
            children.push_back(iTog);
            dy += padding + iTog->h;
            auto uTog = make_shared<ui::ToggleButton>(dx, dy, dw, 50, "Upgradable");
            uTog->toggled = options->Upgradable;
            uTog->style.justify = ui::Style::JUSTIFY::LEFT;
            uTog->events.toggled += [this](bool s){options->Upgradable = s; upate_event();};
            //children.push_back(uTog);
            //s->add(uTog);
            //dy += padding + uTog->h;
            auto unTog = make_shared<ui::ToggleButton>(dx, dy, dw, 50, "Not Installed");
            unTog->toggled = options->NotInstalled;
            unTog->style.justify = ui::Style::JUSTIFY::LEFT;
            unTog->events.toggled += [this](bool s){options->NotInstalled = s; upate_event();};
            s->add(unTog);
            children.push_back(unTog);
            dy += padding + unTog->h;
            auto descTog = make_shared<ui::ToggleButton>(dx,dy,dw,50, "Search Descriptions");
            descTog->toggled = options->NotInstalled;
            descTog->style.justify = ui::Style::JUSTIFY::LEFT;
            descTog->events.toggled += [this](bool s){options->SearchDescription = s; upate_event();};
            children.push_back(descTog);
            s->add(descTog);
            dy += padding + descTog->h;
            if(!options->Repos.empty()) {
                //TODO: set height of the list based on number of entries
                _repoList = make_shared<ListBox>(dx, dy, dw, 200, 25, s);
                for(auto &[r, set]: options->Repos){
                    auto item = _repoList->add(r);
                    if(set) {
                        item->_selected = true;
                        _repoList->selectedItems.emplace(item);
                    }
                }
                std::sort(_repoList->contents.begin(), _repoList->contents.end());
                _repoList->mark_redraw();
                _repoList->events.selected += [this](const shared_ptr<ListBox::ListItem>& li){ options->Repos[li->label] = true; upate_event(); };
                _repoList->events.deselected += [this](const shared_ptr<ListBox::ListItem>& li){ options->Repos[li->label] = false; upate_event(); };
                children.push_back(_repoList);
                s->add(_repoList);
                dy += padding + _repoList->h;
            }
            if(!options->Licenses.empty()) {
                _licenseList = make_shared<ListBox>(dx, dy, dw, 200, 25, scene);
                for(auto &[l, set] : options->Licenses) {
                    auto item = _licenseList->add(l);
                    if(set) {
                        item->_selected = true;
                        _licenseList->selectedItems.emplace(item);
                    }
                }
                children.push_back(_licenseList);
                s->add(_licenseList);
                _licenseList->mark_redraw();
                _licenseList->events.selected += [this](const shared_ptr<ListBox::ListItem>& li){ options->Licenses[li->label] = true; upate_event(); };
                _licenseList->events.deselected += [this](const shared_ptr<ListBox::ListItem>& li){ options->Licenses[li->label] = false; upate_event(); };
            }
            return s;
        }
    };

    class FilterButton:public RoundImageButton{
    public:
        RoundCornerStyle style;
        shared_ptr<FilterOptions> options;
        FilterButton(int x, int y, int w, int h, shared_ptr<FilterOptions> defaultOptions, RoundCornerStyle style = RoundCornerStyle()) : RoundImageButton(x, y, w, h, ICON(assets::png_filter_png), style) {
            options = defaultOptions;
        }
        PLS_DEFINE_SIGNAL(FILTER_EVENT, FilterOptions);

        class FILTER_EVENTS: public BUTTON_EVENTS {
        public:
            FILTER_EVENT updated;
        };

        FILTER_EVENTS events;

        void on_overlay_hidden(ui::InnerScene::DialogVisible v){
            //events.updated(options);
        }

        void on_mouse_click(input::SynMotionEvent &ev) override{
            ev.stop_propagation();

            auto ov = new FilterOverlay(x+w,y+h,500,800,options);
            ov->scene->on_hide += [=](auto &d) { on_overlay_hidden(d); };
            ov->events.updated += [=](FilterOptions &o){events.updated(o);};
            ov->show();
            ui::MainLoop::refresh();
        }
    };

    /*
    * _____________________________________________________________________________________
    * | Package info                                                                      |
    * | Package name                                                                      |
    * | Package version                                                                   |
    * | Package etc                                                                       |
    * |                                                                                   |
    * | [Install(Upgrade)] [Uninstall] /\*[Download] [Preview]     [Pending: [+0/-0]] *\/ |
    * ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    */
    class PackageInfoPanel: public RoundCornerWidget{
    public:
        int padding = 5;
        int controlHeight = 40;
        int controlWidth = 200;
        PackageInfoPanel(int x, int y, int w, int h, RoundCornerStyle style, shared_ptr<ui::InnerScene> &scene) : RoundCornerWidget(x,y,w,h,style){
            _text = make_shared<ui::MultiText>(x,y,w,h,"");
            _text->set_coords(x+padding,y+padding,w-(2*padding),h-(2*padding) - controlHeight);
            children.push_back(_text);
            _installBtn = make_shared<EventButton>(x,y,200, controlHeight,"Install");
            _removeBtn = make_shared<EventButton>(x,y,200, controlHeight,"Uninstall");
            _downloadBtn = make_shared<EventButton>(x,y,200, controlHeight,"Download");
            _previewBtn = make_shared<EventButton>(x,y,200, controlHeight,"Preview");
            _actionCounter = make_shared<EventButton>(x,y,200, controlHeight, "Pending: [+0/-0]");
            children.push_back(_installBtn);
            scene->add(_installBtn);
            children.push_back(_removeBtn);
            scene->add(_removeBtn);
            //children.push_back(_downloadBtn);
            //children.push_back(_previewBtn);
            //children.push_back(_actionCounter);
            _installBtn->events.clicked += [this](void*){events.install();};
            _removeBtn->events.clicked += [this](void*){events.uninstall();};
            _downloadBtn->events.clicked += [this](void*){events.download();};
            _previewBtn->events.clicked += [this](void*){events.preview();};
            _actionCounter->events.clicked += [this](void*){events.enact();};
            layout_buttons();
        }

        PLS_DEFINE_SIGNAL(PACKAGE_EVENT, void*);

        class PACKAGE_EVENTS {
        public:
            PACKAGE_EVENT install;
            PACKAGE_EVENT uninstall;
            PACKAGE_EVENT download;
            PACKAGE_EVENT preview;
            PACKAGE_EVENT enact;
        };

        PACKAGE_EVENTS events;

        void on_reflow() override{
            _text->set_coords(x+padding,y+padding,w-(2*padding),h-(2*padding) - controlHeight);
            _text->mark_redraw();
            layout_buttons();
        }

        void set_text(string text){
            _text->undraw();
            _text->text = std::move(text);
            _text->mark_redraw();
            this->mark_redraw();
        }

        void set_states(bool installed, bool canPreview = false, bool showDownload = false){
            if(installed){
                _installBtn->disable();
                _removeBtn->enable();
            }
            else{
                _installBtn->enable();
                _removeBtn->disable();
            }
            if(canPreview)
                _previewBtn->show();
            else
                _previewBtn->hide();
            if(showDownload)
                _downloadBtn->show();
            else
                _downloadBtn->hide();
        }

        void set_actions(int add, int remove){
            stringstream ss;
            ss << "Pending: [+" << add << "/-" << remove << "]";
            _actionCounter->textWidget->text = ss.str();
            _actionCounter->text = ss.str();
            _actionCounter->mark_redraw();
            _actionCounter->textWidget->mark_redraw();
            mark_redraw();
        }
    private:
        shared_ptr<ui::MultiText> _text;
        shared_ptr<EventButton> _installBtn, _removeBtn, _downloadBtn, _previewBtn, _actionCounter;

        void layout_buttons(){
            auto dx = x + padding;
            auto dy = y + h - padding - controlHeight;
            _installBtn->set_coords(dx,dy,controlWidth,controlHeight);
            dx += controlWidth + padding;
            _removeBtn->set_coords(dx,dy,controlWidth,controlHeight);
            dx += controlWidth + padding;
            _downloadBtn->set_coords(dx,dy,controlWidth,controlHeight);
            dx += controlWidth + padding;
            _previewBtn->set_coords(dx,dy,controlWidth,controlHeight);

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
    };
    /*
     * _______________________________________________
     * | Installing %d packages and %d dependencies: |
     * | ___________________________________________ |
     * | | List                                    | |
     * | | Box                                     | |
     * | | Contents                                | |
     * | ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯ |
     * | This will require approximately %d K/MB     |
     * | [OK] [Abort]                                |
     * ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
     */
    class InstallDialog: public widgets::Overlay {
    public:
        InstallDialog(int x, int y, int w, int h, const std::vector<shared_ptr<package>> &toInstall): Overlay(x,y,w,h){
            packages = toInstall;
        }
        void build_dialog() override {
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
            int lh = min((uint)300, ((labels.size() )* (utils::line_height()+5))+10);
            auto l1 = new widgets::ListBox(dx, dy+padding, dw, lh, utils::line_height(), scene);
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
        void setCallback(const function<void(bool)>& callback){
            _callback = callback;
        }
    private:
        function<void(bool)> _callback;
        std::vector<shared_ptr<package>> packages;
        bool _accepted = false;

        void on_button_selected(std::string s) override{
            if(s == "OK") {
                _accepted = true;
                run_install();
                return;
            }
            else {
                _accepted = false;
            }
            widgets::Overlay::on_button_selected(s);
            _callback(_accepted);
        }

        void run_install(){
            auto td = new TerminalDialog(500,500,800,1100, "opkg install");
            td->set_callback([this](){this->_callback(this->_accepted);this->hide();});
            td->show();
            ui::TaskQueue::add_task([=](){
                auto ret = opkg::Install(packages, [=](const string s) { td->stdout_callback(s); });
                if (ret == 0)
                    td->stdout_callback("Done.");
                else
                    td->stdout_callback("Error!");
                std::cout << "opkg install returned with exit code " << ret << std::endl;
            });
        }
    };

    /*
    * _________________________________________________
    * | Uninstalling %d packages and %d dependencies: |
    * | _____________________________________________ |
    * | | List                                      | |
    * | | Box                                       | |
    * | | Contents                                  | |
    * | ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯ |
    * |  [X] Autoremove dependencies                  |
    * |                                               |
    * | This will free approximately %d K/MB          |
    * | [OK] [Abort]                                  |
    * ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    */
    class UninstallDialog: public widgets::Overlay {
    public:
        UninstallDialog(int x, int y, int w, int h, const std::vector<shared_ptr<package>> &toInstall): Overlay(x,y,w,h){
            packages = toInstall;
        }
        void build_dialog() override {
            this->create_scene();
            layout = make_shared<ui::VerticalLayout>(x, y, w, h, this->scene);
            int dx = padding;
            int dy = padding;
            int dw = w - padding - padding;
            t1 = new ui::Text(dx, dy, dw, utils::line_height(), "Loading...");
            layout->pack_start(t1);
            l1 = new widgets::ListBox(dx, dy + padding, dw,  utils::line_height(), utils::line_height(), scene);
            l1->selectable = false;
            layout->pack_start(l1);
            cb = new ui::ToggleButton(dx, dy + padding + padding, dw, utils::line_height(), "Auto-remove dependencies");
            cb->toggled = true;
            cb->events.toggled += [this](bool s){on_autoremove_tick(s);};
            cb->style.justify = ui::Style::JUSTIFY::LEFT;
            layout->pack_start(cb);
            t2 = new ui::Text(dx, dy + padding + padding, dw, utils::line_height(), "");
            layout->pack_start(t2);
            auto button_bar = new ui::HorizontalLayout(0, 0, this->w, 50, this->scene);
            layout->pack_end(button_bar);
            button_bar->y -= 2;

           // layout->reflow();

            this->add_buttons(button_bar);
            ui::TaskQueue::add_task([this](){this->update_texts();});
        }
        void on_reflow() override{
            l1->on_reflow();
        }
        void mark_redraw() override{
            layout->refresh();
        }
        void setCallback(const function<void(bool)>& callback){
            _callback = callback;
        }
    private:
        function<void(bool)> _callback;
        const int padding = 20;
        shared_ptr<ui::VerticalLayout> layout;
        ui::Text *t1 = nullptr;
        ListBox *l1 = nullptr;
        ui::Text *t2 = nullptr;
        ui::ToggleButton *cb = nullptr;
        bool _accepted = false;
        std::vector<shared_ptr<package>> packages;
        bool dependencies = true;

        void on_button_selected(std::string s) override{
            if(s == "OK") {
                _accepted = true;
                run_uninstall();
                return;
            }
            else
                _accepted = false;
            widgets::Overlay::on_button_selected(s);
            _callback(_accepted);
        }

        void on_autoremove_tick(bool state){
            dependencies = state;
            ui::TaskQueue::add_task([this](){this->update_texts();});
        }

        void add_buttons_reflow(ui::HorizontalReflow *button_bar) {
            auto default_fs = ui::Style::DEFAULT.font_size;
            for (auto b: this->buttons) {
                auto image = stbtext::get_text_size(b, default_fs);

                button_bar->pack_start(new ui::DialogButton(20, 0, image.w + default_fs, 50, this, b));
            }
        }

        vector<shared_ptr<package>> results;

        void run_uninstall(){
            auto td = new TerminalDialog(500,500,800,1100, "opkg uninstall");
            td->set_callback([this](){this->_callback(this->_accepted);this->hide();});
            td->show();
            auto ret = opkg::Uninstall(packages, [td](const string s){td->stdout_callback(s);}, dependencies ? " --autoremove" : "");
            if(ret == 0)
                td->stdout_callback("Done.");
            else
                td->stdout_callback("Error!");
            std::cout << "opkg uninstall returned with exit code " << ret << std::endl;
        }

        void update_texts(){
            if(!t1 || !t2 || !l1)
                return;
            results.clear();
            auto ret = opkg::Instance->ComputeUninstall(packages, dependencies, &results);
            if(ret != 0){
                printf("OPKG ERROR! %d\n", ret);
            }
            stringstream s1;
            s1 << "Uninstalling " << packages.size() << " packages and ";
            s1 << results.size() - packages.size() << " dependencies:";

            t1->set_text(s1.str());

            uint totalSize = 0;
            int lh = min((uint)300, ((results.size() )* (utils::line_height()+5))+10);
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
            Overlay::mark_redraw();
        }
    };
}