//
// Created by brant on 2/5/24.
//

#pragma once

#include "list_box.h"
#include "keyboard.h"
#include "overlay.h"
#include <utility>
#include "../../src/widget_helpers.h"

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
            this->title = title;
        }
        void build_dialog() override;
        void on_reflow() override;
        void mark_redraw() override;
        void stdout_callback(const std::string &s);
        void set_callback(const std::function<void()> &cb);
    private:
        const int padding = 20;
        shared_ptr<ui::VerticalLayout> layout;
        ui::Text *t1 = nullptr;
        ui::MultiText *l1 = nullptr;
        std::deque<std::string> consoleBuffer;
        int buffer_size = 16;
        std::function<void()> callback;

        void on_button_selected(std::string s) override;

        void add_buttons_reflow(ui::HorizontalReflow *button_bar);

        void push_line(const std::string &l);


        void update_texts();
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

        void on_reflow() override;

        void on_mouse_click(input::SynMotionEvent &ev) override;
    private:
        widgets::Keyboard *_keyboard;
        shared_ptr<ui::Pixmap> pixmap;

        void onChange(KeyboardEvent ev);
        void onDone(KeyboardEvent ev);
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

        void show();
        void hide() override;
        void render() override;
        void mark_redraw() override;

        PLS_DEFINE_SIGNAL(OMENU_EVENT, MenuData*);

        class OMENU_EVENTS {
        public:
            OMENU_EVENT updated;
        };

        OMENU_EVENTS events;

    private:
        void upate_event();
        void refresh_event(void*);
        /*
         * [x] Check upgrades
         * UI Style:
         * (*) Cosy
         * ( ) Compact
         * Font Size [10[^v]]
         */
        ui::Scene make_overlay();
    };

    class ConfigButton : public RoundImageButton {
    public:
        RoundCornerStyle style;
            MenuData *data;
        ConfigButton(int x, int y, int w, int h, MenuData *data, RoundCornerStyle style = RoundCornerStyle()) : RoundImageButton(x, y, w, h, ICON(assets::png_menu_png), style) {
            this->data = data;
        }
        void on_overlay_hidden(ui::InnerScene::DialogVisible v);
        void on_mouse_click(input::SynMotionEvent &ev) override;
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

        void show() override;

        void render() override;

        void mark_redraw() override;

        PLS_DEFINE_SIGNAL(OFILTER_EVENT, FilterOptions);

        class OFILTER_EVENTS {
        public:
            OFILTER_EVENT updated;
        };

        OFILTER_EVENTS events;

    private:
        void upate_event();
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
        ui::Scene make_overlay();
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

        void on_overlay_hidden(ui::InnerScene::DialogVisible v);
        void on_mouse_click(input::SynMotionEvent &ev) override;
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

        void on_reflow() override;
        void set_text(string text);
        void set_states(bool installed, bool canPreview = false, bool showDownload = false);
        void set_actions(int add, int remove);
    private:
        shared_ptr<ui::MultiText> _text;
        shared_ptr<EventButton> _installBtn, _removeBtn, _downloadBtn, _previewBtn, _actionCounter;

        void layout_buttons();
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
            pinned = true;
        }
        void build_dialog() override;
        void setCallback(const function<void(bool)>& callback);
    private:
        function<void(bool)> _callback;
        std::vector<shared_ptr<package>> packages;
        bool _accepted = false;

        void on_button_selected(std::string s) override;
        void run_install();
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
            pinned = true;
        }
        void build_dialog() override;
        void on_reflow() override;
        void mark_redraw() override;
        void setCallback(const function<void(bool)>& callback);

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
        bool dependencies = false;

        void on_button_selected(std::string s) override;
        void on_autoremove_tick(bool state);
        void add_buttons_reflow(ui::HorizontalReflow *button_bar);

        vector<shared_ptr<package>> results;

        void run_uninstall();
        void update_texts();
    };
}