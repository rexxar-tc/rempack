//
// Created by brant on 9/14/24.
//

#pragma once
#include <UI/StatefulWidget.h>

class RempackState;

class Rempack : public rmlib::StatefulWidget<Rempack> {
public:
    static RempackState createState();
};

class RempackState : public rmlib::StateBase<Rempack> {

};
