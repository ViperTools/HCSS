#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "misc-no-recursion"

#include "ComponentValueParser.hpp"
#include "../Util/util.hpp"
#include <utility>

/**
 * @brief Consumes the next ComponentValue
 * 
 * @return The next ComponentValue
 */

ComponentValue ComponentValueParser::consume() {
    auto temp = std::move(values.front());
    values.pop_front();
    return temp;
}

/**
 * @brief Consumes a token of TokenType 'type' and if the token does not match, throws 'error'
 * 
 * @param type The next token's expected type
 * @param error The error message to throw if the token type is invalid
 * @return Token The token with TokenType 'type' on success
 * @return Throws error 'error' on failure
 */

Token ComponentValueParser::consume(TokenType type, const string& error) {
    auto t = consume<Token>();
    if (t.type == type) {
        return t;
    }
    SYNTAX_ERROR(error, t);
}

/**
 * @brief Peeks the next ComponentValue
 * 
 * @return The next ComponentValue on success
 * @return nullopt on failure
 */

optional<ComponentValue> ComponentValueParser::peek(int idx) {
    return !values.empty() && idx < values.size() ? optional<ComponentValue>(values[idx]) : nullopt;
}

/**
 * @brief Returns a boolean representing whether the token at 'idx' matches the given type
 * 
 * @param type The type to match
 * @param idx The index of the token to check (default 0)
 * @return true If the token at 'idx' matches the given type
 * @return false If the token at 'idx' does not match the given type
 */

bool ComponentValueParser::check(TokenType type, int idx) {
    auto t = peek<Token>(idx);
    return t && t -> type == type;
}

bool ComponentValueParser::check(const wstring& lexeme, int idx) {
    auto t = peek<Token>(idx);
    return t && t -> lexeme == lexeme;
}

bool ComponentValueParser::check(const wchar_t& lexeme, int idx) {
    auto t = peek<Token>(idx);
    return t && t -> type == DELIM && t -> lexeme[0] == lexeme;
}

/**
 * @brief Gets the mirror of an opening TokenType
 * 
 * @param type The opening TokenType (e.g LEFT_BRACKET)
 * @return TokenType The closing TokenType (e.g RIGHT_BRACKET) on success
 * @return Throws an error if an invalid TokenType was specified
 */
 
TokenType ComponentValueParser::mirror(TokenType type) {
    switch (type) {
        case LEFT_BRACE: return RIGHT_BRACE;
        case LEFT_BRACKET: return RIGHT_BRACKET;
        case LEFT_PAREN: return RIGHT_PAREN;
        default: SYNTAX_ERROR("Expected {, [, or (", nullopt);
    }
}