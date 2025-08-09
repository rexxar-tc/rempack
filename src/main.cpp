#include <iostream>
#include <unistd.h>

#include "rempack.h"
#include "ScreenCatcher.h"

#include <thread>
#include <sys/wait.h>

int main() {
#ifndef CAPTURE_SCREEN
    Rempack::.startApp();
#else
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("couldn't open pipe!");
        return 1;
    }

#ifdef DEV
    auto t = std::thread([=](){
        auto ret = ScreenCatcher::Listen(pipefd[0]);
        std::cerr << "ScreenCatcher returned with code " << ret << std::endl;
        close(pipefd[0]);
    });
    //t.detach();

    Rempack::startApp(pipefd[1]);
    std::cerr<<"CLOSING PIPE" << std::endl;
    close(pipefd[1]);
    t.join();
    std::cout << "DONE!" << std::endl;

    return 0;
#else
    auto pid = fork();
    if (pid == -1) {
        perror("error forking!");
        return 1;
    }

    if (pid == 0) {
        close(pipefd[1]);
        auto sc = ScreenCatcher();
        auto ret = sc.Listen(pipefd[0]);
        std::cerr << "ScreenCatcher returned with code " << ret << std::endl;
        close(pipefd[0]);
        return ret;
    } else {
        close(pipefd[0]);
        Rempack::startApp(pipefd[1]);
        std::cerr<<"CLOSING PIPE" << std::endl;
        close(pipefd[1]);
        waitpid(pid, nullptr, 0);
        std::cout << "DONE!" << std::endl;
    }
#endif
#endif
}