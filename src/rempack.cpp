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
#include "ScreenCatcher.h"
#include <filesystem>
#include "debugging.h"
namespace fs = filesystem;
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

int sPipe;

#ifdef DEV
// run this constructor as early as possible to preempt calls to framebuffer::get()
// and inject a custom framebuffer instead of the default file-backed RM2 size buffer
__attribute__((constructor(1000)))
static void my_fb_initializer() {
    std::cout << "init fb: " << (framebuffer::_FB == nullptr) << std::endl;
    //set memory-backed framebuffer of any dimension
    framebuffer::_FB = make_shared<framebuffer::VirtualFB>(1404,1872);
    //framebuffer::_FB = make_shared<framebuffer::VirtualFB>(1872,1404);
}
#endif

void Rempack::startApp(int pipe){
    sPipe = pipe;
    startApp();
}

void setupStyle(){
    setenv("RMKIT_DEFAULT_FONT", "/usr/share/fonts/ttf/ebgaramond/EBGaramond-VariableFont_wght.ttf", 0);
    stbtext::GRAYSCALE = true;
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

void initScreen(bool clear = true){
    fb->update_mode = UPDATE_MODE_FULL;
    fb->waveform_mode = WAVEFORM_MODE_INIT;
    fb->redraw_screen(true);
    if(clear)
        fb->clear_screen();
}

static string get_cached_path(const string& basename = "rempack"){
    const char* xdg_cache_home = std::getenv("XDG_CACHE_HOME");
    string base = xdg_cache_home ? xdg_cache_home : std::getenv("HOME") + std::string("/.cache");
    string path =  base + "/" + basename;
    if(!fs::exists(path))
        fs::create_directories(path);
    return path;
}

int scount = 0;
volatile bool sigExit = false;

void onExit(int signal){
    if(sigExit || sPipe <= 0){
        return;
    }
    //close the screencap process if it's running
    close(sPipe);
    sPipe = -1;
    //std::cerr << v << "SIGNAL: " << signal << std::endl;
    //attempt to wake main thread and let it clean up
    sigExit = true;
    ui::TaskQueue::wakeup();
    ui::IdleQueue::wakeup();
}

string spath;

string screenPath(int idx){
    if(spath.empty()) {
        std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm tm{};
        localtime_r(&t, &tm);
        std::ostringstream oss;
        oss << "rempack/screens/" << std::put_time(&tm, "%Y-%m-%d_%H-%M") << "/";
        spath = get_cached_path(oss.str());
        if (!fs::exists(spath))
            fs::create_directories(spath);
    }
    stringstream sss;
    sss << spath << std::setfill('0') << std::setw(3) << scount << ".png";

    return sss.str();
}

void capture_screen(int idx){
#ifdef CAPTURE_SCREEN
    ScreenCatcher::WriteScreen(screenPath(idx), fb->fbmem, fb->width, fb->height, sPipe);
#endif
}

void capture_layers(int idx){
#ifdef CAPTURE_LAYERS
    debugging::render_debug_layers(ui::MainLoop::scene, fb->width, fb->height, fs::path(screenPath(idx)).replace_extension(), sPipe);
#endif
}

void Rempack::startApp() {
    ui::MainLoop::exit += onExit;
    setupStyle();

    fb = framebuffer::get();
    auto scene = buildHomeScene(fb->width, fb->height);
    ui::MainLoop::set_scene(scene);

    //TODO: we need one tick before the first real frame to set some things up?
    ui::MainLoop::main();
    ui::MainLoop::refresh();
    //ui::MainLoop::redraw();

#ifdef DEV
    capture_screen(scount);
    capture_layers(scount);
    scount++;
#endif
    setupDebug();
#ifdef DEV
    capture_screen(scount);
    capture_layers(scount);
    scount++;
    auto *lastframe = new uint8_t[fb->byte_size];
#endif

    filterMgr->updateLists(filterOpts, "");
    while(true) {
        auto mstart = chrono::steady_clock::now();
        ui::MainLoop::main();
        auto dirty = fb->dirty;
#ifdef DEV
        //I really don't know why this is necessary sometimes.
        dirty = memcmp(lastframe, fb->fbmem, fb->byte_size) != 0;
#endif
        if (dirty) {
#ifndef NDEBUG
            auto dmt = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - mstart);
            std::cout << "main loop time: " << dmt.count() << "ms" << std::endl;
#endif
            capture_screen(scount);
            capture_layers(scount);
            scount++;
            if (scount % 40 == 0) {
                initScreen(false);
            }
#ifdef DEV
            memcpy(lastframe, fb->fbmem, fb->byte_size);
            fb->reset_dirty(fb->dirty_area);
            fb->dirty = 0;
#endif
        }

        ui::MainLoop::redraw();
        //fb->waveform_mode = WAVEFORM_MODE_GC16;
        ui::MainLoop::read_input();

        if(sigExit){
            std::cerr << "BRK\n";
            break;
        }
    }
    std::cerr << "MAIN LOOP EXIT" << std::endl;
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
    filterMgr->updateLists(filterOpts, currentQuery);
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

    std::deque<std::function<void(void)>> debug_steps;
void setupDebug() {
//#ifndef NDEBUG
    //std::raise(SIGINT);   //firing a sigint here helps synchronize remote gdbserver
    //sleep(10);

    //std::filesystem::remove_all("/home/root/.cache/rempack");

    debug_steps.emplace_back([&]() {
        std::cout << "STEP 1\n";
        packagePanel->select("splashscreen-batteryempty-starr");
        displayBox->get_preview();
    });
    debug_steps.emplace_back([&]() {
        std::cout << "STEP 2\n";
        packagePanel->select("");
    });
    debug_steps.emplace_back([&]() {
        std::cout << "STEP 3\n";
        packagePanel->select("dotnet-host");
    });
    debug_steps.emplace_back([&]() {
        std::cout << "STEP 4\n";
        packagePanel->select("splashscreen-batteryempty-chaotic_ribbon");
        displayBox->get_preview();
    });
    debug_steps.emplace_back([=]() {
        sigExit = true;
        ui::TaskQueue::wakeup();
    });

    auto tptr = ui::TimerList::get()->set_interval([&]() {
        if (!debug_steps.empty()) {
            ui::IdleQueue::add_task([&](){
                    std::cout << "STEP" << std::endl;
                    auto &step = debug_steps.front();
                    debug_steps.pop_front();
                    step();
                });
        }
    }, 500);
    //auto ev = input::SynMotionEvent();
    //    ev.x = searchBox->x;
    //    ev.y = searchBox->y;
    //    ev.left = 1;

    //searchBox->on_mouse_click(ev);
    //_selected = pk;
    //onInstallClick(nullptr);
    //auto pt = opkg::DownloadPackage(pk, dummyline);
    //std::cout << pt << std::endl;

//auto scene = ui::make_scene();
//auto ed = make_shared<widgets::RoundCornerEditor>(20,20,300,300);
//    scene->add(ed);
//    for(const auto &c : ed->children) {
//        scene->add(c);
//        for(const auto &sc : c->children) {
//            scene->add(sc);
//        }
//    }
//    scene->pinned = true;
//    ui::MainLoop::show_overlay(scene);
//#endif
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
    filterPanel = new widgets::ListBox(0, 0, 300, applicationPane->h, 45, widgets::LightButtonStyle());
    std::vector<std::string> sections;
    pkg.LoadSections(&sections);
    for (const auto &s: sections)
        filterPanel->add(s);

    filterPanel->events.selected += PLS_DELEGATE(onFilterAdded);
    filterPanel->events.deselected += PLS_DELEGATE(onFilterRemoved);

    packagePanel = new widgets::ListBox(padding, 0, layout->w - filterPanel->w - padding, applicationPane->h, 45, widgets::LightButtonStyle());
    packagePanel->multiSelect = false;
    packagePanel->events.selected += PLS_DELEGATE(onPackageSelect);
    packagePanel->events.deselected += PLS_DELEGATE(onPackageDeselect);

    filterMgr = new ListFilter(filterPanel, packagePanel);
    filterMgr->updateLists(filterOpts, "");

    displayBox = new widgets::PackageInfoPanel(0,0,applicationPane->w,applicationPane->h, widgets::RoundCornerStyle());

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
