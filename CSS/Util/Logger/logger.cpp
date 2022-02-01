#include "logger.hpp"
#include "Stopwatch.hpp"
#include <iostream>
using std::string;
using std::pair;
using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::milliseconds;

double Logger::totalTime;
Stopwatch Logger::stopwatch;

std::map<LogType, pair<string, int>> colors = {
    { DEBUG, { "DEBUG", 33 }},
    { ERROR, { "ERROR", 31 }},
    { OUT, { "OUTPUT", 0 }}
};

void Logger::log(std::string str, LogType type) {
    std::cout << "\33[" + std::to_string(colors[type].second) + "m[" + colors[type].first + "]\33[0m " + str << std::endl;
}

void Logger::call(std::string task, std::function<void()> f) {
    auto start = high_resolution_clock::now();
    f();
    duration<double, std::milli> ms = high_resolution_clock::now() - start;
    log(task + " COMPLETED IN " + std::to_string(ms.count()) + "ms", DEBUG);
    totalTime += ms.count();
}

void Logger::lap() {
    log("TOTAL TIME: " + std::to_string(totalTime), DEBUG);
    totalTime = 0;
}