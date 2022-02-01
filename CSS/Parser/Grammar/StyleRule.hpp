#pragma once

#include <utility>

#include "Selector.hpp"
#include "StyleBlock.hpp"

class StyleRule {
    public:
        COMPLEX_SELECTOR_LIST selectors;
        STYLE_BLOCK block;
        explicit StyleRule(COMPLEX_SELECTOR_LIST selectors, STYLE_BLOCK block = {})
            : selectors(std::move(selectors)),
            block(std::move(block))
        {};
};