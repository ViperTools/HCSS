#include "logger.hpp"
#include <iostream>
using std::string;
using std::pair;

std::map<LogType, std::pair<std::string, int>> colors = {
        { LOG_OUTPUT, { "OUTPUT", 0 }},
        { LOG_DEBUG, { "DEBUG", 93 }},
        { LOG_SUCCESS, { "SUCCESS", 32 }},
        { LOG_FAIL, { "FAIL", 31 }},
        { LOG_WARNING, { "WARNING", 33 }},
        { LOG_ERROR, { "ERROR", 31 }}
};

void Logger::log(std::string str, LogType type) {
    std::cout << "\33[" + std::to_string(colors[type].second) + "m[" + colors[type].first + "]\33[0m " + str << std::endl;
}