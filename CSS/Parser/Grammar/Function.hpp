#pragma once

#include "../Types.hpp"
#include <utility>
#include <vector>

struct FunctionCall {
    Token name;
    std::vector<std::vector<ComponentValue>> arguments;
    FunctionCall(Token name, std::vector<std::vector<ComponentValue>> arguments = {})
        : name(std::move(name)),
        arguments(std::move(arguments))
    {};
};

struct FunctionDefinition {
    Token name;
    std::vector<std::pair<wstring, std::vector<ComponentValue>>> parameters = {};
    explicit FunctionDefinition(Token name)
        : name(std::move(name))
    {};
};