#pragma once

#include "../Types.hpp"
#include "SimpleBlock.hpp"
#include <utility>
#include <vector>
#include <optional>

struct AtRule {
    Token name;
    std::vector<ComponentValue> prelude;
    std::optional<SimpleBlock> block;
    explicit AtRule(Token name, std::vector<ComponentValue> prelude = {}, std::optional<SimpleBlock> block = std::nullopt)
        : name(std::move(name)),
        prelude(std::move(prelude)),
        block(std::move(block))
    {};
};