//
// Created by brant on 1/24/24.
//
//#define DEBUG_FB

#include <rmkit.h>
#include <unordered_set>
#include <utility>
#include "rempack.h"
#include "widgets.h"
#include "debug/debug_widgets.h"
#include "../opkg/opkg.h"
#include "display/list_box.h"
#include "rempack/rempack_widgets.h"
#include "platform_rules.h"
#include "ListFilter.h"
using ListItem = widgets::ListBox::ListItem;

ui::Scene buildHomeScene(int width, int height);

opkg pkg;

widgets::SearchBox *searchBox;
widgets::ListBox *filterPanel, *packagePanel;
widgets::PackageInfoPanel *displayBox;
shared_ptr<framebuffer::FB> fb;
widgets::MenuData *menuData;
std::string currentQuery;

ListFilter *filterMgr;
void setupDebug();
shared_ptr<package> selected;
shared_ptr<widgets::FilterOptions> filterOpts;

void setupStyle(){
    setenv("RMKIT_DEFAULT_FONT", "/usr/share/fonts/ttf/ebgaramond/EBGaramond-VariableFont_wght.ttf", 0);
    ui::Style::DEFAULT = {
            .font_size = 40,
            .line_height = 1.0,
            .underline = false,
            .justify = ui::Style::CENTER,
            .valign = ui::Style::VALIGN::MIDDLE,
            .border_top = false,
            .border_left = false,
            .border_bottom = false,
            .border_right = false
    };
}

[[noreturn]]
void Rempack::startApp() {
    setupStyle();
    fb = framebuffer::get();
    auto scene = buildHomeScene(fb->width, fb->height);
    ui::MainLoop::set_scene(scene);

    ui::MainLoop::main();
    ui::MainLoop::refresh();
    //ui::MainLoop::redraw();

    setupDebug();
    filterMgr->updateLists(filterOpts, "");
    while(true){
        ui::MainLoop::main();
        ui::MainLoop::redraw();
        fb->waveform_mode = WAVEFORM_MODE_GC16;
        //fb->update_mode = UPDATE_MODE_PARTIAL;
        ui::MainLoop::read_input();
    }

}

void searchQueryOpen(string s){
    if(selected != nullptr){
        selected = nullptr;
        for(const auto &p : packagePanel->selectedItems)
            p->_selected = false;
        packagePanel->selectedItems.clear();
        packagePanel->mark_redraw();
        displayBox->display_package(nullptr);
    }
}
void searchQueryUpdate(string s){
    currentQuery = std::move(s);
    filterMgr->updateLists(filterOpts, currentQuery);
}
void onFilterAdded(shared_ptr<ListItem> item) { // NOLINT(*-unnecessary-value-param)
    filterOpts->Sections.emplace(item->label);
    filterMgr->updateLists(filterOpts, currentQuery);
}
void onFilterRemoved(shared_ptr<ListItem> item) { // NOLINT(*-unnecessary-value-param)
    filterOpts->Sections.erase(item->label);
    filterMgr->updateLists(filterOpts, currentQuery);
}
void onPackageSelect(shared_ptr<ListItem> item) { // NOLINT(*-unnecessary-value-param)
    auto pk = any_cast<shared_ptr<package>>(item->object);
    std::cout << "Package selected: " << pk->Package << "\n";
    selected = pk;
    displayBox->display_package(pk);
}
void onPackageDeselect([[maybe_unused]] shared_ptr<ListItem> item) {
    //auto pk = any_cast<shared_ptr<package>>(item->object);
    //printf("Package deselected: %s\n", pk->Package.c_str());
    selected = nullptr;
    displayBox->display_package(nullptr);
}
void onFiltersChanged(widgets::FilterOptions &options){
    //_filterOpts = options;
    if(options.groupSplash)
        packagePanel->sortPredicate = platform::RemarkableRules::splashscreenComparator;
    else
        packagePanel->sortPredicate = nullptr;
    packagePanel->mark_redraw();
}

void onInstallClick(void*){
    auto m = new widgets::InstallDialog(500,500,600,800,vector<shared_ptr<package>>{selected});

    m->setCallback([](bool b){displayBox->display_package(selected);});
    if(selected->Package.rfind("splashscreen") == 0) {
        auto conf = platform::rules.checkSplashConflicts(pkg, selected);
        if (!conf.empty()) {
            for (const auto &c: conf) {
                std::cout << "CONFLICT: " << c->Package << std::endl;
            }
            auto cd = new widgets::ConflictDialog(500, 500, 600, 800, selected, conf);
            cd->setCallback([m](bool accept) {
                if (accept)
                    m->show();
                return;
            });
            cd->show();
            return;
        }
    }
    m->show();
 }
void onUninstallClick(void*){
    auto m = new widgets::UninstallDialog(500,500,600,800,vector<shared_ptr<package>>{selected});
    m->setCallback([](bool b){displayBox->display_package(selected);});
    m->show();
}
void onPreviewClick(void*){
    displayBox->set_image(selected);
}

void initScreen(){
    fb->draw_rect(0,0,fb->width, fb->height, BLACK);
    //fb->update_mode = UPDATE_MODE_FULL;
    //fb->waveform_mode = WAVEFORM_MODE_A2;
    fb->dirty = true;
    fb->redraw_screen();
    fb->clear_screen();
    //fb->redraw_screen();
    //fb->update_mode = UPDATE_MODE_PARTIAL;
    //fb->waveform_mode = WAVEFORM_MODE_GC16;
}

void setupDebug(){
#ifndef NDEBUG
    std::raise(SIGINT);   //firing a sigint here helps synchronize remote gdbserver
    //sleep(10);

    //packagePanel->select("splashscreen-poweroff-sacks_spiral");
    auto ev = input::SynMotionEvent();
        ev.x = searchBox->x;
        ev.y = searchBox->y;
        ev.left = 1;

    searchBox->on_mouse_click(ev);
    //_selected = pk;
    //onInstallClick(nullptr);
    //auto pt = opkg::DownloadPackage(pk, dummyline);
    //std::cout << pt << std::endl;
#endif
}

//1404x1872 - 157x209mm -- 226dpi
ui::Scene buildHomeScene(int width, int height) {
    int padding = 20;
    auto scene = ui::make_scene();

    initScreen();

    //vertical stack that takes up the whole screen
    auto layout = new ui::VerticalReflow(padding, padding, width - padding*2, height - padding*2, scene);

    opkg::Instance = &pkg;
    pkg.InitializeRepositories();
    /* Search + menus */
    //short full-width pane containing search and menus
    auto searchPane = new ui::HorizontalReflow(0, 0, layout->w, 80, scene);

    filterOpts = make_shared<widgets::FilterOptions>(widgets::FilterOptions{
            .Installed = true,
            .Upgradable = false,
            .NotInstalled = true,
            .SearchDescription = true,
            .SearchHidden = true,
            .groupSplash = false,
    });
    for(auto &r : pkg.repositories){
        filterOpts->Repos.emplace(r, r != "entware");   //hide entware by default, there's so many openwrt packages it drowns out toltec
    }
    auto filterButton = new widgets::FilterButton(0,0,60,60, filterOpts);
    filterButton->events.updated += onFiltersChanged;
    menuData = new widgets::MenuData;
    auto settingButton = new widgets::ConfigButton(padding*2, 0, 60, 60, menuData);
    searchBox = new widgets::SearchBox(padding, 0, layout->w - 120 - padding * 2, 60, widgets::RoundCornerStyle());
    searchBox->events.updated += PLS_DELEGATE(searchQueryUpdate);
    searchBox->events.open += PLS_DELEGATE(searchQueryOpen);
    searchBox->events.done += PLS_DELEGATE(searchQueryUpdate);
    searchPane->pack_start(filterButton);
    searchPane->pack_start(searchBox);
    searchPane->pack_start(settingButton);

    /* Applications */
    //full-width horizontal stack underneath the search pane. give it half the remaining height
    auto applicationPane = new ui::HorizontalReflow(0, 0, layout->w, (layout->h - searchPane->h - padding)/2, scene);
    filterPanel = new widgets::ListBox(0, 0, 300, applicationPane->h, 45, scene);
    std::vector<std::string> sections;
    pkg.LoadSections(&sections);
    for (const auto &s: sections)
        filterPanel->add(s);

    filterPanel->events.selected += PLS_DELEGATE(onFilterAdded);
    filterPanel->events.deselected += PLS_DELEGATE(onFilterRemoved);

    packagePanel = new widgets::ListBox(padding, 0, layout->w - filterPanel->w - padding, applicationPane->h, 45, scene);
    packagePanel->multiSelect = false;
    packagePanel->events.selected += PLS_DELEGATE(onPackageSelect);
    packagePanel->events.deselected += PLS_DELEGATE(onPackageDeselect);

    filterMgr = new ListFilter(filterPanel, packagePanel);
    filterMgr->updateLists(filterOpts, "");

    displayBox = new widgets::PackageInfoPanel(0,0,applicationPane->w,applicationPane->h, widgets::RoundCornerStyle(), scene);

    displayBox->events.install += PLS_DELEGATE(onInstallClick);
    displayBox->events.uninstall += PLS_DELEGATE(onUninstallClick);
    displayBox->events.preview += PLS_DELEGATE(onPreviewClick);

    layout->pack_start(searchPane);
    layout->pack_start(applicationPane);
    applicationPane->pack_start(filterPanel);
    applicationPane->pack_start(packagePanel);
    layout->pack_end(displayBox);

    layout->reflow();

    return scene;
}
