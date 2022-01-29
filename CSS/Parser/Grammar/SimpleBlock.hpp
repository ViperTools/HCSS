#pragma once

#include "../Macros.hpp"
#include <vector>
#include <optional>

class SimpleBlock {
    public:
        Token open;
        std::vector<COMPONENT_VALUE> value;
        std::optional<Token> close;
        SimpleBlock(Token open, std::vector<COMPONENT_VALUE> value = {}, std::optional<Token> close = std::nullopt)
            : open(open),
            value(value),
            close(close)
        {};
};