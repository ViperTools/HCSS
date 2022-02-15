#pragma once

#include "../Types.hpp"
#include <utility>
#include <vector>

struct FunctionCall {
    Token name;
    std::vector<ComponentValue> value;
    explicit FunctionCall(Token name, std::vector<ComponentValue> value = {})
        : name(std::move(name)),
        value(std::move(value))
    {};
};

struct FunctionDefinition {
    Token name;
    std::map<wstring, std::vector<ComponentValue>> parameters = {};
    explicit FunctionDefinition(Token name)
        : name(std::move(name))
    {};
};