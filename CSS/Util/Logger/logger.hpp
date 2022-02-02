#pragma once
#include <string>
#include <iostream>
#include <map>

enum LogType {
    DEBUG,
    ERROR,
    OUT
};

struct Logger {
    static void log(std::string str, LogType type = OUT);
};