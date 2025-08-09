//
// Created by brant on 7/29/25.
//

/*
 * This manages recording the screen in the background(ish).
 * The main program will fork this into a new process and push a frame into the pipe
 * after each render call. That will push it into a worker thread at a low priority.
 *
 * This is an incredibly overcomplicated system to compress PNGs in a
 * low-priority thread entirely to avoid a very minor stutter on the UI thread while recording the screen.
 *
 * This is apparently how this is done in C++
 *
 * We must create a new process and have it increase its niceness value.
 * Niceness is more or less priority, but it only applies to processes, not threads.
 *
 * Our main thread parses incoming data before handing it off to one of the worker threads.
 * Two worker threads because my device has two cores
 * Worker threads pop images off the stack and lay them down on disk.
 *
 * Writing the PNGs involves allocating several MB of memory and running some crappy compression
 * It takes a few seconds to process a full frame, hence the low priority workers.
 */

//we need the STB libs
#include <rmkit.h>
#include "ScreenCatcher.h"

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <vector>
#include <filesystem>

#ifdef DEV
#define CONCURRENCY 8
#else
#define CONCURRENCY 2
#endif

//try to keep memory use under control during very rapid render calls
constexpr int  MAX_QUEUE_SIZE = (16 * CONCURRENCY);

static_assert(sizeof(remarkable_color) == sizeof(uint16_t), "Screen capture is not implemented for this platform! Wrong color width!");

namespace fs = std::filesystem;
struct shot{
    size_t w;
    size_t h;
    std::vector<remarkable_color> data;
    std::string path;
};

std::queue<shot> workQueue;
std::mutex queueMutex;
std::condition_variable cv;
bool done = false;

static void dump_screen(const std::string &path, const vector<uint16_t> &fb, uint w, uint h) {
    auto buf = (unsigned char *) malloc(w * h * 3);
    auto llen = buf;
    for (int i = 0; i < w * h; i++) {
        auto s = fb[i];
        *(llen++) = uint8_t(((s & 0xf800) >> 11) / 31. * 255.);
        *(llen++) = uint8_t(((s & 0x7e0) >> 5) / 63. * 255.);
        *(llen++) = uint8_t((s & 0x1f) / 31. * 255.);
    }
    //std::cerr << "SAVING " << path << std::endl;

    auto start = std::chrono::steady_clock::now();
    //set compression low so it's faster
    //there's basically no difference in output size between level 1 and 9
    //but higher compression takes at least a second longer
    //TODO: See about replacing the png compressor
    stbi_write_png_compression_level = 1;
    stbi_write_png(path.c_str(), w, h, 3, buf, w * 3);
    stbi_write_png_compression_level = 8;
    free(buf);

    auto dmt = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start);

    std::cout << "saved " << dmt.count() << "ms:  " << path << std::endl;
}


void worker() {
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [&] { return !workQueue.empty() || done; });

        if (done && workQueue.empty()) {
            break;
        }

        if (done) {
            std::cerr << "CLEARING WORK ON EXIT" << std::endl;
        }

        do {
            if (workQueue.empty())
                break;
            auto data = std::move(workQueue.front());
            workQueue.pop();
            lock.unlock();

            fs::path path = data.path;
            path = path.parent_path();
            if (!fs::exists(path))
                fs::create_directories(path);

            dump_screen(data.path, data.data, data.w, data.h);
            lock.lock();
        } while (done && !workQueue.empty());
        lock.unlock();
        if(done)
            return;
    }
}

int read_exact(int fd, void* buf, uint32_t count) {
    int total = 0;
    char* ptr = static_cast<char*>(buf);

    while (total < count) {
        auto n = read(fd, ptr + total, count - total);
        if (n <= 0) {
            std::cerr << "REOF" << std::endl;
            // Error or EOF
            return n;
        }
        total += n;
    }
    return total; // total == count here
}

int ScreenCatcher::Listen(int pipe) {
    //compressing pngs is rather slow, so de-prioritize this process
    //otherwise the UI thread can stutter
    nice(10);

    std::vector<std::thread> workers(CONCURRENCY);
    for(int i = 0; i < CONCURRENCY; i++){
        workers.emplace_back(worker);
    }

    int err = 0;
    size_t buflen = 0;
    int n = read_exact(pipe, &buflen, sizeof(size_t));
    //std::cout << "plen " << buflen << '\n';
    if(n != sizeof(size_t)){
        std::cerr << "SIZE ERROR" << std::endl;
    }
    while (n >= 0) {
        std::string path;
        path.resize(buflen);;
        n = read_exact(pipe, path.data(), buflen);
        //std::cout << path << '\n';
        if (n <= 0) {
            err = 1;
            break;
        }

        size_t dims[2];
        n = read_exact(pipe, dims, sizeof(size_t) * 2);
        // std::cout << "dims " << dims[0] << ',' << dims[1] << '\n';
        if (n <= 0) {
            err = 2;
            break;
        }

        std::vector<remarkable_color> buf(dims[0] * dims[1]);
        n = read_exact(pipe, buf.data(), dims[0] * dims[1] * sizeof(remarkable_color));
        //std::cout << "read " << buf.size() << std::endl;
        if (n <= 0){
            err = 3;
            break;
        }

        shot data {
                dims[0],
                dims[1],
                std::move(buf),
                std::move(path)
        };
        size_t sz;
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            workQueue.emplace(data);
            sz = workQueue.size();
        }
        cv.notify_one();

        auto sts = std::chrono::steady_clock::now();
        //if the work queue is full, we want the calling thread to block
        //easiest way is to block this thread so the pipe backs up
        while (sz >= MAX_QUEUE_SIZE) {
            std::unique_lock<std::mutex> lock(queueMutex);
            sz = workQueue.size();
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); //each image takes 1-2 seconds to process
        }
        auto dts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - sts);
        if(dts.count() > 10)
            std::cout << "STALL: " << dts.count() << std::endl;
        n = read_exact(pipe, &buflen, sizeof(size_t));
        if(n <= 0){
            err = 9;
            break;
        }
    }

    std::cerr << "EXITING" << std::endl;
    done = true;
    cv.notify_all();
    for(auto &w : workers) {
        if(w.joinable())
        w.join();
    }
    return err;
}

ssize_t write_all(int fd, const void* buf, size_t count) {
    const char* ptr = static_cast<const char*>(buf);
    ssize_t total_written = 0;

    while (total_written < count) {
        ssize_t n = write(fd, ptr + total_written, count - total_written);
        if (n < 0) {
            std::cerr << "WEOF" << std::endl;
            if (errno == EINTR) continue;
            return -1;  // error
        }
        total_written += n;
    }
    return total_written;
}

int ScreenCatcher::WriteScreen(const std::string& path, remarkable_color *buf, uint w, uint h, int pipe) {
    if(done)
        return 0;

    //std::cout << "WRITE " << path << '\n';
    size_t len = path.length();
    auto n = write_all(pipe, &len, sizeof(len));
    if(n != sizeof(size_t))
        return 4;
    n = write_all(pipe, path.data(), len);
    if(n!=len)
        return 5;
    size_t dims[2] = {w,h};
    n = write_all(pipe, dims, sizeof(size_t) * 2);
    if(n != sizeof(size_t) * 2)
        return 6;
    auto buflen = w * h * sizeof(remarkable_color);
    n = write_all(pipe, buf, buflen);
    if(n != buflen)
        return 7;
    return 0;
}
