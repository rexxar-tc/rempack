//
// Created by brant on 7/6/25.
//

#include "dispatcher.h"
#include <mutex>

namespace widgets {
    std::deque<std::function<void()>> Dispatcher::tasks;
    std::mutex Dispatcher::taskLock;
    void Dispatcher::add_task(const std::function<void()>& t) {
        std::lock_guard<std::mutex> lock(taskLock);
        tasks.push_back(t);
    }

    void Dispatcher::run_tasks() {
        if (tasks.empty()) {
            return;
        }
        std::deque<std::function<void()>> localTasks;
        {
            std::lock_guard<std::mutex> lock(taskLock);
            std::swap(localTasks, tasks);
        }
        for (const auto &task: localTasks) {
            try {
                task();
            } catch (const std::exception& e) {
                std::cerr << "Dispatcher exception! " << e.what() << std::endl;
            }
        }
    }
} // widgets