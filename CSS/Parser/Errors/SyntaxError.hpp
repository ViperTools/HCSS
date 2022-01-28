#pragma once

#include "../../Lexer/Token.hpp"
#include <locale>
#include <codecvt>

using std::to_string;

class SyntaxError : public std::exception {
    public:
        string error;
        SyntaxError(Token* tok, const string& error)
            : error(tok != nullptr ? "\nSyntax Error:\nLine: " + to_string(tok->line) + "\nPosition: " + to_string(tok->position) + "\nLexeme: " + std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(tok->lexeme) + "\nType: " + to_string(tok->type) + "\nDetails: " + error : "\nSyntax Error:\nDetails: " + error)
        {};
        [[nodiscard]] const char* what() const noexcept override {
            return error.c_str();
        }
};