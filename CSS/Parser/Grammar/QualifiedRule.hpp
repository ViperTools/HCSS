#pragma once

#include "../Types.hpp"
#include "SimpleBlock.hpp"
#include <utility>
#include <vector>
#include <optional>

struct QualifiedRule {
    std::vector<ComponentValue> prelude;
    std::optional<SimpleBlock> block;
    explicit QualifiedRule(std::vector<ComponentValue> prelude = {}, std::optional<SimpleBlock> block = std::nullopt)
        : prelude(std::move(prelude)),
        block(std::move(block))
    {};
};