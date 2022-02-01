#pragma once

#include "../Macros.hpp"
#include "SimpleBlock.hpp"
#include <utility>
#include <vector>
#include <optional>

class QualifiedRule {
    public:
        std::vector<COMPONENT_VALUE> prelude;
        std::optional<SimpleBlock> block;
        explicit QualifiedRule(std::vector<COMPONENT_VALUE> prelude = {}, std::optional<SimpleBlock> block = std::nullopt)
            : prelude(std::move(prelude)),
            block(std::move(block))
        {};
};