#pragma once

#include "../ParserMacros.hpp"
#include <vector>
#include <optional>

class QualifiedRule {
    public:
        std::vector<COMPONENT_VALUE> components;
        std::optional<SimpleBlock> block;
        QualifiedRule(std::vector<COMPONENT_VALUE> components = {}, std::optional<SimpleBlock> block = std::nullopt)
            : components(components),
            block(block)
        {};
};