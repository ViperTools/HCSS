#pragma once
#include <string>

template<typename T>
struct CallResult {
    T result;
    double time;
    void output(std::string taskName);
    CallResult(T result, double time)
        : result(result),
        time(time)
    {};
};