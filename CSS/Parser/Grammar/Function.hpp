#pragma once

#include "../ParserMacros.hpp"
#include <vector>

class Function {
    public:
        Token name;
        std::vector<ComponentValue> components;
        Function(Token name, std::vector<ComponentValue> components = {})
            : name(name),
            components(components)
        {};

};