#pragma once

#include <variant>
#include "../Lexer/Token.hpp"
#include "../Util/util.hpp"

class AtRule;
class Function;
class QualifiedRule;
class SimpleBlock;
class StyleRule;

using SyntaxNode = std::variant<std::monostate, AtRule, Function, QualifiedRule, SimpleBlock, StyleRule>;
using ComponentValue = variant_append<SyntaxNode, Token>;