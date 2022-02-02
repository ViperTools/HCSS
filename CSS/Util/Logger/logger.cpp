#include "logger.hpp"
#include <iostream>
using std::string;
using std::pair;

std::map<LogType, std::pair<std::string, int>> colors = {
        { DEBUG, { "DEBUG", 33 }},
        { ERROR, { "ERROR", 31 }},
        { OUT, { "OUTPUT", 0 }}
};

void Logger::log(std::string str, LogType type) {
    std::cout << "\33[" + std::to_string(colors[type].second) + "m[" + colors[type].first + "]\33[0m " + str << std::endl;
}