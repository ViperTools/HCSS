#pragma once

#include <variant>
#include "../Lexer/Token.hpp"

class AtRule;
class Function;
class QualifiedRule;
class SimpleBlock;
class StyleRule;

#define SYNTAX_NODE std::variant<AtRule, Function, QualifiedRule, SimpleBlock, StyleRule>
#define COMPONENT_VALUE std::variant<SYNTAX_NODE, Token>