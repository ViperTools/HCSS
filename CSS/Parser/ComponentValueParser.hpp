#pragma once

#include "Errors/SyntaxError.hpp"
#include "../Lexer/Token.hpp"
#include <utility>
#include <vector>
#include <variant>
#include <deque>
#include "Grammar/AtRule.hpp"
#include "Grammar/Function.hpp"
#include "Grammar/QualifiedRule.hpp"
#include "Grammar/SimpleBlock.hpp"
#include "Grammar/Selector.hpp"
#include "Grammar/StyleBlock.hpp"
#include "Grammar/StyleRule.hpp"
#include "Types.hpp"
using std::vector;
using std::deque;

#define SYNTAX_ERROR(s, t) throw SyntaxError(s, t, __LINE__, __FILE__)

struct ComponentValueParser {
    explicit ComponentValueParser(vector<ComponentValue> vec)
    {
        std::move(vec.begin(), vec.end(), std::back_inserter(values));
    };
    explicit ComponentValueParser(deque<ComponentValue> values)
        : values(std::move(values))
    {};
    explicit ComponentValueParser(const vector<Token>& tokens)
    {
        std::move(tokens.begin(), tokens.end(), std::back_inserter(values));
    };
    deque<ComponentValue> values;
    ComponentValue consume();
    template<typename T = ComponentValue> T consume();
    Token consume(TokenType type, const string& error);
    optional<ComponentValue> peek(int idx = 0);
    template<typename T = ComponentValue> optional<T> peek(int idx = 0);
    template<typename T = ComponentValue> bool check();
    bool check(TokenType type, int idx = 0);
    bool check(const wstring& lexeme, int idx = 0);
    bool check(const wchar_t& lexeme, int idx = 0);
    TokenType mirror(TokenType type);
    inline void skipws() {
        if (check(WHITESPACE))
            values.pop_front();
    };
};

/**
 * @brief Consumes the next SyntaxNode of type T
 * 
 * @tparam T The more specific type of the SyntaxNode (e.g Token, SimpleBlock, etc.)
 * @return SyntaxNode of type T on success
 * @return Throws an error on failure
 */

template<typename T>
T ComponentValueParser::consume() {
    if (auto val = std::get_if<T>(&values.front())) {
        auto temp = std::move(*val);
        values.pop_front();
        return temp;
    }
    else {
        SYNTAX_ERROR(string("Called consume method with an invalid type. The specified type (") + string(typeid(T).name()).substr(1) + ") does not match the next token's type.", nullopt);
    }
}

/**
 * @brief Peeks the next SyntaxNode of type T
 * 
 * @tparam T The more specific type of the SyntaxNode (e.g Token, SimpleBlock, etc.)
 * @return SyntaxNode of type T on success
 * @return nullopt on failure
 */

template<typename T>
optional<T> ComponentValueParser::peek(int idx) {
    if (values.empty() || idx > values.size()) return nullopt;
    if (const T* val = std::get_if<T>(&values[idx])) {
        return *val;
    }
    return nullopt;
}

/**
 * @brief Checks if the next SyntaxNode is of type T
 * 
 * @tparam T The more specific type of the SyntaxNode (e.g Token, SimpleBlock, etc.)
 * @return True if the SyntaxNode matches the type, false if it does not
 */

template<typename T>
bool ComponentValueParser::check() {
    return peek<T>().has_value();
}