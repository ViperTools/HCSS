#pragma once
#include <chrono>
using std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;
using std::chrono::duration;
using std::chrono::milliseconds;

struct Stopwatch {
    double elapsed = 0;
    time_point last;
    bool active;
    bool start();
    double stop();
    double lap();
};