#pragma once

#include "../Types.hpp"
#include <utility>
#include <vector>
#include <optional>
using std::move;

struct SimpleBlock {
    Token open;
    std::vector<ComponentValue> value;
    std::optional<Token> close;
    explicit SimpleBlock(Token open, std::vector<ComponentValue> value = {}, std::optional<Token> close = std::nullopt)
        : open(move(open)),
        value(std::move(value)),
        close(move(close))
    {};
};