#pragma once

#include "../Types.hpp"
#include <utility>
#include <vector>

struct Function {
    Token name;
    std::vector<ComponentValue> value;
    explicit Function(Token name, std::vector<ComponentValue> value = {})
        : name(std::move(name)),
        value(std::move(value))
    {};
};