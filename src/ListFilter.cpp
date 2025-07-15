//
// Created by brant on 7/14/25.
//

#include "ListFilter.h"
#include "algorithm/boyer_moore.h"
using ListItem = widgets::ListBox::ListItem;
namespace boyer = strings::boyer_moore;

std::vector<shared_ptr<ListItem>> sections;
std::vector<shared_ptr<widgets::ListBox::ListItem>> querySet;
std::vector<shared_ptr<widgets::ListBox::ListItem>> pkCache;

void ListFilter::updateLists(const shared_ptr<widgets::FilterOptions>& options, const string &query) {
    //TODO: shove this into a background task.
    //the time cost here is not *huge* but it's also not trivial
    //on dual core devices, this can safely be delegated to another thread
    //otherwise we delegate it to the start of the next frame, freeing the UI immediately
    unordered_set<string> querySections;
    querySections.reserve(sections.size());
    auto start = std::chrono::steady_clock::now();
    if (!query.empty()) {
        querySet.clear();
        sections.clear();
        boyer::pattern pat;
        boyer::init_pattern(query, pat);
        for (const auto &it : pkCache) {
            auto pk = any_cast<shared_ptr<package>>(it->object);
            std::vector<size_t> indexes = boyer::search(pk->Package, pat);
            if (indexes.empty()) {
                if (!options->SearchDescription)
                    continue;
                indexes = boyer::search(pk->Description, pat);
                if (indexes.empty())
                    continue;
            }
            querySections.emplace(pk->Section);
            querySet.push_back(it);
        }
    } else {
        if (querySet.size() != pkCache.size()) {
            querySet = pkCache;
            //sections = opkg::Instance->sections;
        }
    }

    querySections.erase("");
    vector<shared_ptr<ListItem>> filterSet;
    filterSet.reserve(querySet.size());

    for (const auto &it: querySet) {
        auto pk = any_cast<shared_ptr<package>>(it->object);
        if (!options->Sections.empty() && options->Sections.find(pk->Section) == options->Sections.end())
            continue;
        if (!options->SearchHidden || query.empty()) {
            if (!options->Licenses.empty() && !options->Licenses.find(pk->License)->second)
                continue;
            if (!options->Repos.empty() && !options->Repos.find(pk->Repo)->second)
                continue;
        }
        if (!((options->Installed && pk->IsInstalled()) ||
              (options->NotInstalled && pk->State == package::NotInstalled)))
            continue;
        if (options->Upgradable && !pk->Upgradable())
            continue;
        filterSet.push_back(it);
    }

    //TODO: put this into the widget dispatcher
    packagePanel->contents = std::move(filterSet);
    packagePanel->mark_redraw();

    unordered_set<string> erases;
    unordered_set<string> current;
    for(const auto &sec : filterPanel->contents){
        current.emplace(sec->label);
        if(sec->_selected)
            continue;
        if(CONTAINS(querySections, sec->label))
            continue;
        if(CONTAINS(options->Sections, sec->label))
            continue;
        erases.emplace(sec->label);
    }

    for(const auto &er : erases){
        filterPanel->remove(er);
    }

    for(const auto &nsc : sections){
        if(!CONTAINS(current, nsc->label)){
            filterPanel->contents.push_back(nsc);
        }
    }
    filterPanel->mark_redraw();

    auto stop = std::chrono::steady_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Sort time: " << dt.count() << "ms\n";

}

ListFilter::ListFilter(widgets::ListBox *filterPan, widgets::ListBox *packagePan) :
        filterPanel(filterPan), packagePanel(packagePan){
    for (const auto &[n, pk]: opkg::Instance->packages) {
        //ListBox will trim strings internally depending on render width
        string displayName = pk->Package;
        displayName.append(" -- ").append(pk->Description);
        displayName.erase(std::remove(displayName.begin(), displayName.end(), '\n'), displayName.end());
        auto item = packagePanel->add(displayName, displayName, pk);
        pkCache.push_back(item);
    }
    std::vector<std::string> lsec;
    opkg::Instance->LoadSections(&lsec);
    std::sort(lsec.begin(), lsec.end());
    for (const auto &s: lsec)
        sections.push_back(filterPanel->add(s));
}
