#pragma once

#include "Selector.hpp"
#include "StyleBlock.hpp"

class StyleRule {
    public:
        COMPLEX_SELECTOR_LIST selectors;
        STYLE_BLOCK block;
        StyleRule(COMPLEX_SELECTOR_LIST selectors, STYLE_BLOCK block = {})
            : selectors(selectors),
            block(block)
        {};
};