#pragma once

#include "../Types.hpp"
#include <utility>
#include <vector>
#include <optional>
using std::move;

struct SimpleBlock {
    Token open;
    std::vector<ComponentValue> value = {};
    std::optional<Token> close;
};