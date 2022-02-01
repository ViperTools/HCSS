#pragma once

#include "../Macros.hpp"
#include "SimpleBlock.hpp"
#include <utility>
#include <vector>
#include <optional>

class AtRule {
    public:
        Token name;
        std::vector<COMPONENT_VALUE> prelude;
        std::optional<SimpleBlock> block;
        explicit AtRule(Token name, std::vector<COMPONENT_VALUE> prelude = {}, std::optional<SimpleBlock> block = std::nullopt)
            : name(std::move(name)),
            prelude(std::move(prelude)),
            block(std::move(block))
        {};
};