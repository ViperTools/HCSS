#pragma once

#include <variant>
#include "../Lexer/Token.hpp"

class AtRule;
class Function;
class QualifiedRule;
class SimpleBlock;

#define SYNTAX_NODE std::variant<AtRule, Function, QualifiedRule, SimpleBlock>
#define COMPONENT_VALUE std::variant<SYNTAX_NODE, Token>