#pragma once

#include "../Lexer/Token.hpp"
#include <vector>
#include <variant>
using std::vector;

#include "Grammar/AtRule.hpp"
#include "Grammar/Function.hpp"
#include "Grammar/QualifiedRule.hpp"
#include "Grammar/SimpleBlock.hpp"
#include "ParserMacros.hpp"

#define Reconsume idx--
#define Last tokens[idx - 1]

class Parser {
    public:
        vector<Token> tokens;
        vector<SYNTAX_NODE> rules;
        vector<SYNTAX_NODE> Parse();
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
        AtRule ConsumeAtRule();
        Function ConsumeFunction();
        QualifiedRule ConsumeQualifiedRule();
        SimpleBlock ConsumeSimpleBlock();
        COMPONENT_VALUE ConsumeComponentValue();
        vector<SYNTAX_NODE> ConsumeRulesList();
};