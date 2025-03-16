#pragma once

#include "../Types.hpp"
#include <utility>
#include <vector>

struct FunctionCall {
    Token name;
    std::vector<std::vector<ComponentValue>> arguments;
};

struct FunctionDefinition {
    Token name;
    std::vector<std::pair<wstring, std::vector<ComponentValue>>> parameters = {};
};