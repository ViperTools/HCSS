#pragma once

#include "../Macros.hpp"
#include <optional>
#include <vector>

struct Variable {
    Token dollar;
    Token name;
    Variable(Token dollar, Token name)
        : dollar(dollar),
        name(name)
    {};
};

struct VariableDeclaration {
    Variable variable;
    Token eq;
    std::vector<COMPONENT_VALUE> value;
    VariableDeclaration(Variable variable, Token eq, std::vector<COMPONENT_VALUE> value)
        : variable(variable),
        eq(eq),
        value(value)
    {};
};