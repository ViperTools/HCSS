#pragma once

#include "Token.hpp"
#include "SyntaxNode.hpp"
#include <vector>
#include <memory>
using std::vector;
using std::unique_ptr;
#define snptr unique_ptr<SyntaxNode>

class Parser {
    public:
        vector<Token> tokens;
        vector<snptr> rules;
        vector<snptr> Parse();
        Parser(vector<Token> tokens)
            : tokens(tokens)
        {};
    private:
        int idx = 0;
        bool top = true;
        Token* Peek();
        void Ignore();
        bool Check(TokenType type);
        Token Consume();
        Token Consume(TokenType type, string error);
        snptr ConsumeAtRule();
        snptr ConsumeFunction();
        snptr ConsumeQualifiedRule();
        snptr ConsumeSimpleBlock();
        COMPONENT_VALUE ConsumeComponentValue();
        vector<snptr> ConsumeRulesList();
};