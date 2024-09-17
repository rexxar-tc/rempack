//
// Created by brant on 9/14/24.
//

#pragma once
#include <UI.h>
#include <UI/Widget.h>
#include <UI/StatefulWidget.h>

#include "ui/layouts.h"
using namespace rmlib;

class RempackState;

class Rempack : public rmlib::StatefulWidget<Rempack> {
public:
    static RempackState createState();
};

class RempackState : public rmlib::StateBase<Rempack> {
public:
    //~RempackState();

    //void init(rmlib::AppContext& ctx, const rmlib::BuildContext& buildCtx);
    auto build(rmlib::AppContext &ctx, const rmlib::BuildContext &buildCtx) const {
        const Canvas* background = nullptr;
        std::optional<Size> backgroundSize = {};
        return buildHomeLayout();
    }
    void checkLandscape(rmlib::AppContext& ctx);
};
