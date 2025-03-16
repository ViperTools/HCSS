#pragma once

#include "../../Lexer/Token.hpp"
#include <locale>
#include <codecvt>
#include <optional>

using std::to_string;

inline std::string wstring_convert(const std::wstring& wstr) {
    std::size_t len = std::wcstombs(nullptr, wstr.c_str(), 0) + 1;
    char* buffer = new char[len];
    std::wcstombs(buffer, wstr.c_str(), len);
    std::string result(buffer);
    delete[] buffer;
    return result;
}

class SyntaxError : public std::exception {
    public:
        string error;
        explicit SyntaxError(const string& error, std::optional<Token> tok = std::nullopt, int line = -1, const string& file = "NULL")
            : error((tok ? "\nSyntax Error:\nLine: " + to_string(tok->line) + "\nColumn: " + to_string(tok->column) + "\nLexeme: " + wstring_convert(tok->lexeme) + "\nType: " + to_string(tok->type) + "\nDetails: " + error : "\nSyntax Error:\nDetails: " + error) + "\nThrown At:\nLine: " + to_string(line) + "\nFile: " + file)
        {};
        [[nodiscard]] const char* what() const noexcept override {
            return error.c_str();
        }
};