#pragma once
#include "Stopwatch.hpp"
#include "CallResult.hpp"
#include "logger.hpp"
#include <string>
#include <utility>
#include <functional>
using std::string;

struct Task {
    std::string name;
    Stopwatch stopwatch;
    std::map<string, double> tasks;
    CallResult<nullptr_t> call(std::string task, std::function<void()> f);
    template <typename T>
    CallResult<T> call(std::string task, std::function<T()> f);
    string str();
    void log();
    explicit Task(std::string name)
        : name(std::move(name))
    {};
};

template <typename T>
CallResult<T> Task::call(string task, std::function<T()> f) {
    stopwatch.start();
    T res = f();
    double ms = stopwatch.lap();
    stopwatch.active = false;
    tasks[task] = ms;
    return { res, task, ms };
}

CallResult<nullptr_t> Task::call(string task, std::function<void()> f) {
    stopwatch.start();
    f();
    double ms = stopwatch.lap();
    stopwatch.active = false;
    tasks[task] = ms;
    return { nullptr, std::move(task), ms };
}

string Task::str() {
    string s = "\n[" + name + "] COMPLETED IN " + std::to_string(stopwatch.elapsed) + "ms";
    for (auto const& [key, val] : tasks) {
        s += "\n    [" + key + "] COMPLETED IN " + std::to_string(val) + "ms";
    }
    return s;
}

void Task::log() {
    Logger::log(str(), DEBUG);
}

template <typename T>
string CallResult<T>::str() {
    return task + " COMPLETION TIME: " + std::to_string(time) + "ms";
}

template <typename T>
void CallResult<T>::log(const string& taskName) {
    Logger::log(str(), DEBUG);
}