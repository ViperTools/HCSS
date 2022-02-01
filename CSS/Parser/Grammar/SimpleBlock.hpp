#pragma once

#include "../Macros.hpp"
#include <utility>
#include <vector>
#include <optional>

class SimpleBlock {
    public:
        Token open;
        std::vector<COMPONENT_VALUE> value;
        std::optional<Token> close;
        explicit SimpleBlock(Token open, std::vector<COMPONENT_VALUE> value = {}, std::optional<Token> close = std::nullopt)
            : open(std::move(open)),
            value(std::move(value)),
            close(std::move(close))
        {};
};