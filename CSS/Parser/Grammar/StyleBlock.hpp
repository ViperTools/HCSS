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

#define RULE variant<AtRule, QualifiedRule>
#define STYLE_BLOCK std::vector<std::variant<Declaration, RULE>>