#pragma once

#include "Token.hpp"
#include "SyntaxNodeVisitor.hpp"
#include <variant>
#include <vector>
#include <memory>

// A component value is one of the preserved tokens, a function, or a simple block.
// #define COMPONENT_VALUE std::variant<unique_ptr<SimpleBlock>, Token*, unique_ptr<Function>>
#define COMPONENT_VALUE std::variant<std::unique_ptr<SyntaxNode>, Token>

class SyntaxNode {
    public:
        virtual void accept(SyntaxNodeVisitor visitor) {};
};