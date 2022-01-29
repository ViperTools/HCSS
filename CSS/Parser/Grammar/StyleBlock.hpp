#pragma once

#include "../Macros.hpp"
#include <vector>
#include <variant>

class Declaration {
    public:
        Token name;
        Token colon;
        std::vector<COMPONENT_VALUE> value;
        bool important;
        Declaration(Token name, Token colon, std::vector<COMPONENT_VALUE> value = {}, bool important = false)
            : name(name),
            colon(colon),
            value(value),
            important(important)
        {};
};

#define RULE_TYPES AtRule, QualifiedRule
#define RULE variant<RULE_TYPES>
#define STYLE_BLOCK_VARIANT std::variant<std::monostate, Declaration, RULE_TYPES>
#define STYLE_BLOCK std::vector<STYLE_BLOCK_VARIANT>