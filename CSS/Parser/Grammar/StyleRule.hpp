#pragma once

#include <utility>

#include "Selector.hpp"
#include "StyleBlock.hpp"

struct StyleRule {
    ComplexSelectorList selectors;
    StyleBlock block;
};