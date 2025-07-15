//
// Created by brant on 7/6/25.
//

#pragma once

#include <functional>
#include <deque>
#include <iostream>
#include <mutex>

namespace widgets {
    class Dispatcher {
    private:
        static std::deque<std::function<void()>> tasks;
        static std::mutex taskLock;
    public:
        static void add_task(const std::function<void()>& t);
        static void run_tasks();
    };
} // widgets
