#pragma once

#include "../Macros.hpp"
#include <vector>

class SimpleBlock {
    public:
        Token open;
        std::vector<COMPONENT_VALUE> components;
        SimpleBlock(Token open, std::vector<COMPONENT_VALUE> components = {})
            : open(open),
            components(components)
        {};
};