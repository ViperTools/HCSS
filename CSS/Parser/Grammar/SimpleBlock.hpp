#pragma once

#include "../Macros.hpp"
#include <vector>

class SimpleBlock {
    public:
        Token open;
        std::vector<COMPONENT_VALUE> value;
        SimpleBlock(Token open, std::vector<COMPONENT_VALUE> value = {})
            : open(open),
            value(value)
        {};
};