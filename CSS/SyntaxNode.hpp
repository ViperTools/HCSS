#pragma once

#include "Token.hpp"
#include "SyntaxNodeVisitor.hpp"

class SyntaxNode {
    public:
        vector<Token> tokens;
        virtual void accept(SyntaxNodeVisitor visitor) {};
};