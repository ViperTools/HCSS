#pragma once

#include <variant>
#include "../Lexer/Token.hpp"

class AtRule;
class Function;
class QualifiedRule;
class SimpleBlock;

#define SyntaxNode std::variant<AtRule, Function, QualifiedRule, SimpleBlock>
#define ComponentValue std::variant<SyntaxNode, Token>