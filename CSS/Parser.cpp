#include "Parser.hpp"
#include "SyntaxError.hpp"
#include "TokenType.hpp"
#include <iostream>
#include "Grammar/AtRule.hpp"
#include "Grammar/Function.hpp"
#include "Grammar/QualifiedRule.hpp"
#include "Grammar/SimpleBlock.hpp"
using std::unique_ptr;
using std::make_unique;

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
        throw SyntaxError(NULL, "No tokens left to consume.");
    }
    idx++;
    return *t;
}

#define Reconsume idx--
#define Last tokens[idx - 1]

vector<snptr> Parser::Parse() {
    rules = ConsumeRulesList();
    return rules;
}

vector<snptr> Parser::ConsumeRulesList() {
    vector<snptr> rules;
    /* while (Peek() != NULL) {
        Token t = Consume();
        switch (t.type) {
            case WHITESPACE: break;
            case T_EOF: {
                return rules;
            }
            case CDO:
            case CDC: {
                if (!top) {
                    rules.push_back(ConsumeQualifiedRule());
                }
                break;
            }
            case AT_KEYWORD: {
                Reconsume;
                rules.push_back(ConsumeAtRule());
                break;
            }
            default: {
                rules.push_back(ConsumeQualifiedRule());
            }
        }
    } */
    return rules;
}

COMPONENT_VALUE Parser::ConsumeComponentValue() {
    Token t = Consume();
    COMPONENT_VALUE val;
    switch (t.type) {
        case LEFT_BRACE: case LEFT_BRACKET: case LEFT_PAREN: {
            return ConsumeSimpleBlock();
        }
        case FUNCTION: {
            return ConsumeFunction();
        }
        default: {
            return t;
        }
    }
}

snptr Parser::ConsumeAtRule() {
    Token at = Consume(AT_KEYWORD, "Expected AT_KEYWORD");
    vector<COMPONENT_VALUE> components;
    while (Peek() != NULL) {
        Token t = Consume();
        switch (t.type) {
            case T_EOF:
            case SEMICOLON: return make_unique<AtRule>(at, components);
            case LEFT_BRACE: {
                return make_unique<AtRule>(at, components, ConsumeSimpleBlock().release());
            }
            default: {
                Reconsume;
                // components.push_back(ConsumeComponentValue());
            }
        }
    }
    return make_unique<AtRule>(at, components);
}



snptr Parser::ConsumeFunction() {
    Token ident = Consume(IDENT, "Expected identifier");
    vector<COMPONENT_VALUE> components;
    while (Peek() != NULL) {
        Token t = Consume();
        switch (t.type) {
            case T_EOF:
            case RIGHT_PAREN: return make_unique<Function>(ident, components);
            default: {
                // components.push_back(ConsumeComponentValue());
            }
        }
    }
    return make_unique<Function>(ident, components);
}

snptr Parser::ConsumeQualifiedRule() {
    vector<COMPONENT_VALUE> components;
    while (Peek() != NULL) {
        Token t = Consume();
        switch (t.type) {
            case T_EOF: return NULL;
            case LEFT_BRACE: {
                return make_unique<QualifiedRule>(components, ConsumeSimpleBlock().release());
            }
            default: {
                Reconsume;
                // components.push_back(ConsumeComponentValue());
            }
        }
    }
    return make_unique<QualifiedRule>(components);
}

snptr Parser::ConsumeSimpleBlock() {
    Token open = Last;
    TokenType mirror;
    if (open.type == LEFT_BRACE) {
        mirror = RIGHT_BRACE;
    }
    else if (open.type == LEFT_BRACKET) {
        mirror = RIGHT_BRACKET;
    }
    else if (open.type == LEFT_PAREN) {
        mirror = RIGHT_PAREN;
    }
    else {
        throw SyntaxError(&open, "Expected {, [, or (");
    }
    vector<COMPONENT_VALUE> components;
    while (Peek() != NULL) {
        Token t = Consume();
        if (t.type == T_EOF || t.type == mirror) {
            return make_unique<SimpleBlock>(open, components);
        }
        else {
            Reconsume;
            // components.push_back(ConsumeComponentValue());
        }
    }
    return make_unique<SimpleBlock>(open, components);
}