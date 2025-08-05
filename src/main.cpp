#include <iostream>
#include <unistd.h>

#include "rempack.h"
#include "ScreenCatcher.h"

#include <thread>

int main() {
#ifndef CAPTURE_SCREEN
    Rempack::.startApp();
#else
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("couldn't open pipe!");
        return 1;
    }

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
        //close(pipefd[1]);
        std::cout << "DONE!" << std::endl;
    }

#endif
}