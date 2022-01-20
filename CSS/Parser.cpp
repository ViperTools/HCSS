#include "Parser.hpp"
#include "SyntaxError.hpp"
#include "TokenType.hpp"
#include <iostream>

Token* Parser::Peek() {
    return idx >= 0 && idx < tokens.size() ? &tokens[idx] : NULL;
}

bool Parser::Check(TokenType type) {
    Token* t = Peek();
    if (t == NULL) {
        return false;
    }
    return t->type == type;
}

Token Parser::Consume(TokenType type, string error) {
    if (!Check(type)) {
        throw SyntaxError(Peek(), error);
    }
    return Consume();
}

Token Parser::Consume() {
    Token* t = Peek();
    if (t == NULL) {
        throw SyntaxError(NULL, "Expected token");
    }
    return *t;
}

void Parser::Parse() {
    
}

void Parser::ConsumeRulesList() {
    while (Peek() != NULL) {
        Token t = Consume();
        switch (t.Type) {
            case WHITESPACE: break;
            case EOF: {
                // Completed parsing. Return rules
                return;
            }
            case CDO:
            case CDC: {
                if (!top) {
                    ConsumeQualifiedRule();
                }
                break;
            }
            case AT_KEYWORD: {
                ConsumeAtRule();
                break;
            }
            default: {
                ConsumeQualifiedRule();
                break;
            }
        }
    }
}