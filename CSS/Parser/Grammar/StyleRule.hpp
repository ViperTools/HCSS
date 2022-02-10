#pragma once

#include <utility>

#include "Selector.hpp"
#include "StyleBlock.hpp"

struct StyleRule {
    ComplexSelectorList selectors;
    StyleBlock block;
    explicit StyleRule(ComplexSelectorList selectors, StyleBlock block = {})
        : selectors(std::move(selectors)),
        block(std::move(block))
    {};
};