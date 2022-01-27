#pragma once

#include "../../Lexer/Token.hpp"
using std::to_string;

class SyntaxError : public std::exception {
    public:
        string error;
        SyntaxError(Token* tok, const string& error)
            : error(tok != nullptr ? "\nSyntax Error:\nLine: " + to_string(tok->line) + "\nPosition: " + to_string(tok->position) + "\nLexeme: " + string(tok -> lexeme.begin(), tok -> lexeme.end()) + "\nType: " + to_string(tok->type) + "\nDetails: " + error : "\nSyntax Error:\nDetails: " + error)
        {};
        [[nodiscard]] const char* what() const noexcept override {
            return error.c_str();
        }
};