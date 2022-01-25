#pragma once

#include "../../Lexer/Token.hpp"
using std::to_string;

class SyntaxError : public std::exception {
    public:
        string error;
        SyntaxError(Token* tok, string error)
            : error(tok != NULL ? "\nSyntax Error:\nLine: " + to_string(tok->line) + "\nPosition: " + to_string(tok->position) + "\nLexeme: " + tok->lexeme + "\nDetails: " + error : "\nSyntax Error:\nDetails: " + error)
        {};
        const char* what() const throw() {
            return error.c_str();
        }
};