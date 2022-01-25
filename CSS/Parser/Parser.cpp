#include "Parser.hpp"
#include "Errors/SyntaxError.hpp"
#include "../Lexer/TokenType.hpp"
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
        throw SyntaxError(NULL, "No tokens left to consume.");
    }
    idx++;
    return *t;
}

vector<SYNTAX_NODE> Parser::Parse() {
    rules = ConsumeRulesList();
    return rules;
}

vector<SYNTAX_NODE> Parser::ConsumeRulesList() {
    vector<SYNTAX_NODE> rules;
    while (Peek() != NULL) {
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
    }
    return rules;
}

COMPONENT_VALUE Parser::ConsumeComponentValue() {
    Token t = Consume();
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

AtRule Parser::ConsumeAtRule() {
    Token at = Consume(AT_KEYWORD, "Expected AT_KEYWORD");
    AtRule rule(at);
    while (Peek() != NULL) {
        Token t = Consume();
        switch (t.type) {
            case T_EOF:
            case SEMICOLON: return rule;
            case LEFT_BRACE: {
                rule.block = ConsumeSimpleBlock();
                return rule;
            }
            default: {
                Reconsume;
                rule.components.push_back(ConsumeComponentValue());
            }
        }
    }
    return rule;
}



Function Parser::ConsumeFunction() {
    Token ident = Consume(IDENT, "Expected identifier");
    Function f(ident);
    while (Peek() != NULL) {
        Token t = Consume();
        switch (t.type) {
            case T_EOF:
            case RIGHT_PAREN: return f;
            default: {
                f.components.push_back(ConsumeComponentValue());
            }
        }
    }
    return f;
}

QualifiedRule Parser::ConsumeQualifiedRule() {
    QualifiedRule rule;
    while (Peek() != NULL) {
        Token t = Consume();
        switch (t.type) {
            case T_EOF: return rule; //NULL;
            case LEFT_BRACE: {
                rule.block = ConsumeSimpleBlock();
                return rule;
            }
            default: {
                Reconsume;
                rule.components.push_back(ConsumeComponentValue());
            }
        }
    }
    return rule;
}

SimpleBlock Parser::ConsumeSimpleBlock() {
    Token open = Last;
    SimpleBlock block(open);
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
            return block;
        }
        else {
            Reconsume;
            block.components.push_back(ConsumeComponentValue());
        }
    }
    return block;
}