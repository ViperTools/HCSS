#pragma once

#include "../ParserMacros.hpp"
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