//
// Created by brant on 9/16/24.
//

#pragma once

#include <UI.h>

using namespace rmlib;

inline auto buildHomeLayout(){
    return Column(
            Row(
            Button("test", Callback()),
            Text("hellorld")
            ),
            Row(
                    Button("test2", Callback()),
                    Text("hello world")
                    )
    );
}