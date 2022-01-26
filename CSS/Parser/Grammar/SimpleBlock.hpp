#pragma once

#include "../ParserMacros.hpp"
#include <vector>

class SimpleBlock {
    public:
        Token open;
        std::vector<ComponentValue> components;
        SimpleBlock(Token open, std::vector<ComponentValue> components = {})
            : open(open),
            components(components)
        {};
};