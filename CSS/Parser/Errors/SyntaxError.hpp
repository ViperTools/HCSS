#pragma once

#include "../../Lexer/Token.hpp"
#include <locale>
#include <codecvt>
#include <optional>

using std::to_string;

class SyntaxError : public std::exception {
    public:
        string error;
        explicit SyntaxError(const string& error, std::optional<Token> tok = std::nullopt, int line = -1, const string& file = "NULL")
            : error((tok.has_value() ? "\nSyntax Error:\nLine: " + to_string(tok->line) + "\nColumn: " + to_string(tok->column) + "\nLexeme: " + std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(tok->lexeme) + "\nType: " + to_string(tok->type) + "\nDetails: " + error : "\nSyntax Error:\nDetails: " + error) + "\nThrown At:\nLine: " + to_string(line) + "\nFile: " + file)
        {};
        [[nodiscard]] const char* what() const noexcept override {
            return error.c_str();
        }
};