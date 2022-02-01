#pragma once

#include "../Macros.hpp"
#include <utility>
#include <vector>

class Function {
    public:
        Token name;
        std::vector<COMPONENT_VALUE> value;
        explicit Function(Token name, std::vector<COMPONENT_VALUE> value = {})
            : name(std::move(name)),
            value(std::move(value))
        {};

};