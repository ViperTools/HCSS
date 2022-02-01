#pragma once
#include "CallResult.hpp"
#include "Stopwatch.hpp"
#include <functional>
#include <string>
#include <map>
#include <chrono>
using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::milliseconds;

enum LogType {
    DEBUG,
    ERROR,
    OUT
};

struct Logger {
    static double totalTime;
    static void log(std::string str, LogType type = OUT);
    static void call(std::string task, std::function<void()> f);
    template <typename T>
    static CallResult<T> call(std::function<T()> f);
    static void lap();
    static Stopwatch stopwatch;
};

template <typename T>
CallResult<T> Logger::call(std::function<T()> f) {
    stopwatch.start();
    T res = f();
    double ms =  stopwatch.lap();
    totalTime += ms;
    return { res, ms };
}

template <typename T>
void CallResult<T>::output(std::string taskName) {
    Logger::log(taskName + " | COMPLETION TIME: " + std::to_string(time) + "ms", DEBUG);
}