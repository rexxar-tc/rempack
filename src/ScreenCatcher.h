//
// Created by brant on 7/29/25.
//

#pragma once

#include <cstdint>
#include <string>

class ScreenCatcher {
public:
    static int Listen(int pipe);
    static int WriteScreen(const std::string& path, uint16_t* buf, uint w, uint h, int pipefd);
};

