#pragma once

#include <variant>
#include "../Lexer/Token.hpp"

class AtRule;
class Function;
class QualifiedRule;
class SimpleBlock;
class StyleRule;
class Variable;
class VariableDeclaration;

#define SYNTAX_NODE_TYPES std::monostate, AtRule, Function, QualifiedRule, SimpleBlock, StyleRule, Variable, VariableDeclaration
#define SYNTAX_NODE std::variant<SYNTAX_NODE_TYPES>
#define COMPONENT_VALUE std::variant<SYNTAX_NODE_TYPES, Token>