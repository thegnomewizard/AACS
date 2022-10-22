#pragma once

#include <functional>
#include <thread>

// Utility class which records the framebuffer and returns h264
class VidCapture {
    std::function<void(int64_t, const uint8_t*, int)> _callback;
    std::thread _thread;
    bool _exit;

    void thread();
public:
    VidCapture();
    ~VidCapture();

    void start(std::function<void(int64_t, const uint8_t*, int)> callback);
    void stop();
};
