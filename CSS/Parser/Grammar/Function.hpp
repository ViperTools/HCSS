#pragma once

#include "../Macros.hpp"
#include <vector>

class Function {
    public:
        Token name;
        std::vector<COMPONENT_VALUE> value;
        Function(Token name, std::vector<COMPONENT_VALUE> value = {})
            : name(name),
            value(value)
        {};

};