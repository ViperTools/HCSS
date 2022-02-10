#pragma once

#include "../Types.hpp"
#include <utility>
#include <vector>
#include <variant>
class StyleRule;

struct Declaration {
    Token name;
    Token colon;
    std::vector<ComponentValue> value;
    bool important;
    Declaration(Token name, Token colon, std::vector<ComponentValue> value = {}, bool important = false)
        : name(std::move(name)),
        colon(std::move(colon)),
        value(std::move(value)),
        important(important)
    {};
};

using StyleBlockVariant = std::variant<std::monostate, Declaration, AtRule, QualifiedRule, StyleRule>;
using StyleBlock = std::vector<StyleBlockVariant>;