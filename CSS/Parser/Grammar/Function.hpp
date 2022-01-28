#pragma once

#include "../Macros.hpp"
#include <vector>

class Function {
    public:
        Token name;
        std::vector<COMPONENT_VALUE> components;
        Function(Token name, std::vector<COMPONENT_VALUE> components = {})
            : name(name),
            components(components)
        {};

};