#pragma once

#include "Token.hpp"
#include "SyntaxNode.hpp"
#include <vector>
using std::vector;

class Parser {
    public:
        vector<Token> tokens;
        vector<SyntaxNode> nodes;
        void Parse();
        Parser(vector<Token> tokens)
            : tokens(tokens)
        {};
    private:
        int idx = 0;
        bool top = true;
        Token* Peek();
        void Ignore();
        void Reconsume();
        bool Check(TokenType type);
        Token Consume();
        Token Consume(TokenType type, string error);
        void ConsumeRulesList();
        void ConsumeAtRule();
        void ConsumeQualifiedRule();
};