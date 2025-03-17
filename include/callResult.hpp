#pragma once
#include <string>

template<typename T>
struct CallResult {
    T result;
    std::string task;
    double time;
    std::string str();
    void log(const std::string& taskName);
    CallResult(T result, std::string task, double time)
        : result(result),
        time(time)
    {};
};