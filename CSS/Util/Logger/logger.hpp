#pragma once
#include <string>
#include <iostream>
#include <map>

enum LogType {
    LOG_OUTPUT,
    LOG_DEBUG,
    LOG_SUCCESS,
    LOG_FAIL,
    LOG_WARNING,
    LOG_ERROR
};

struct Logger {
    static void log(std::string str, LogType type = LOG_OUTPUT);
};