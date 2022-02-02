#pragma once
#include <chrono>
using std::chrono::high_resolution_clock;
using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;
using std::chrono::duration;
using std::chrono::milliseconds;

struct Stopwatch {
    double elapsed = 0;
    time_point last;
    bool active = false;
    bool start();
    double stop();
    double lap();
};

bool Stopwatch::start() {
    if (active) return false;
    active = true;
    last = high_resolution_clock::now();
    return true;
}

double Stopwatch::stop() {
    if (!active) return 0;
    lap();
    active = false;
    return elapsed;
}

double Stopwatch::lap() {
    if (!active) return 0;
    auto temp = last;
    last = high_resolution_clock::now();
    double ms = std::chrono::duration_cast<duration<double, std::milli>>(last - temp).count();
    elapsed += ms;
    return ms;
}