#include "Stopwatch.hpp"

bool Stopwatch::start() {
    if (active) return false;
    active = true;
    last = high_resolution_clock::now();
    return true;
}

double Stopwatch::stop() {
    if (!active) return 0;
    active = false;
    double ms = (high_resolution_clock::now() - last).count();
    elapsed += ms;
    return elapsed;
}

double Stopwatch::lap() {
    if (!active) return 0;
    double ms = (high_resolution_clock::now() - last).count();
    last = high_resolution_clock::now();
    elapsed += ms;
    return ms;
}