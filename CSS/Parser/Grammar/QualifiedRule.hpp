#pragma once

#include "../Macros.hpp"
#include <vector>
#include <optional>

class QualifiedRule {
    public:
        std::vector<COMPONENT_VALUE> prelude;
        std::optional<SimpleBlock> block;
        QualifiedRule(std::vector<COMPONENT_VALUE> prelude = {}, std::optional<SimpleBlock> block = std::nullopt)
            : prelude(prelude),
            block(block)
        {};
};