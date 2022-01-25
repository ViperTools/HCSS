#pragma once

#include "../ParserMacros.hpp"
#include "SimpleBlock.hpp"
#include <vector>
#include <optional>

class AtRule {
    public:
        Token name;
        std::vector<COMPONENT_VALUE> components;
        std::optional<SimpleBlock> block;
        AtRule(Token name, std::vector<COMPONENT_VALUE> components = {}, std::optional<SimpleBlock> block = std::nullopt)
            : name(name),
            components(components),
            block(block)
        {};
};