#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "misc-no-recursion"

#pragma region includes
#include "Macros.hpp"
#include "../Util/util.hpp"
#include "Grammar/Selector.hpp"
#include "Parser.hpp"
#include <deque>
#include <utility>

#include <chrono>
#include <iostream>

using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::milliseconds;

#pragma endregion

#pragma region Helpers

/**
 * @brief Gets the mirror of an opening TokenType
 * 
 * @param type The opening TokenType (e.g LEFT_BRACKET)
 * @return TokenType The closing TokenType (e.g RIGHT_BRACKET) on success
 * @return Throws an error if an invalid TokenType was specified
 */
 
TokenType mirror(TokenType type) {
    switch (type) {
        case LEFT_BRACE: return RIGHT_BRACE;
        case LEFT_BRACKET: return RIGHT_BRACKET;
        case LEFT_PAREN: return RIGHT_PAREN;
        default: SYNTAX_ERROR("Expected {, [, or (", nullopt);
    }
}

#pragma endregion

#pragma region Component Parser

template<>
COMPONENT_VALUE ComponentValueParser::consume() {
    auto temp = values.front();
    values.pop_front();
    return temp;
}

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

template<>
optional<COMPONENT_VALUE> ComponentValueParser::peek(int idx) {
    return !values.empty() && idx < values.size() ? optional<COMPONENT_VALUE>(values[idx]) : nullopt;
}

template<typename T>
optional<T> ComponentValueParser::peek(int idx) {
    if (values.empty() || idx > values.size()) return nullopt;
    if (const T* val = std::get_if<T>(&values[idx])) {
        return *val;
    }
    return nullopt;
}

template<typename T>
bool ComponentValueParser::check() {
    return peek<T>().has_value();
}

bool ComponentValueParser::check(TokenType type, int idx) {
    auto t = peek<Token>(idx);
    return t.has_value() && t -> type == type;
}

bool ComponentValueParser::check(const wstring& lexeme, int idx) {
    auto t = peek<Token>(idx);
    return t.has_value() && t -> lexeme == lexeme;
}

bool ComponentValueParser::check(const wchar_t& lexeme, int idx) {
    auto t = peek<Token>(idx);
    return t.has_value() && t -> type == DELIM && t -> lexeme[0] == lexeme;
}

#pragma endregion

#pragma region Base Parser

/**
 * @brief Parses a base style sheet
 * 
 * @return vector<SYNTAX_NODE> A list of parsed SyntaxNodes
 */

vector<SYNTAX_NODE> BaseParser::parse() {
    rules = consumeRulesList();
    top = false;
    for (auto & i : rules) {
        if (auto rule = std::get_if<QualifiedRule>(&i)) {
            if (rule -> block) {
                i = StyleRule(SelectorParser(rule -> prelude).parse(), StyleBlockParser(rule -> block -> value).parse());
            }
            else {
                i = StyleRule(SelectorParser(rule -> prelude).parse());
            }
        }
    }
    return rules;
}

vector<SYNTAX_NODE> BaseParser::consumeRulesList() {
    vector<SYNTAX_NODE> list;
    while (auto t = peek<Token>()) {
        switch (t -> type) {
            case WHITESPACE: values.pop_front(); break;
            case T_EOF: return list;
            case CDO: case CDC: {
                if (!top) {
                    list.emplace_back(consumeQualifiedRule());
                }
                break;
            }
            case AT_KEYWORD: {
                list.emplace_back(consumeAtRule());
                break;
            }
            default: {
                list.emplace_back(consumeQualifiedRule());
                break;
            }
        }
    }
    return list;
}

COMPONENT_VALUE BaseParser::consumeComponentValue() {
    if (auto t = peek<Token>()) {
        switch (t -> type) {
            case LEFT_BRACE: case LEFT_BRACKET: case LEFT_PAREN: {
                return consumeSimpleBlock();
            }
            case FUNCTION: {
                return consumeFunction();
            }
            default: {
                auto temp = std::move(*t);
                values.pop_front();
                return temp;
            }
        }
    }
    return consume();
}

AtRule BaseParser::consumeAtRule() {
    Token at = consume(AT_KEYWORD, "Expected AT_KEYWORD");
    AtRule rule(at);
    while (!values.empty()) {
        if (auto t = peek<Token>()) {
            switch (t -> type) {
                case T_EOF: case SEMICOLON: values.pop_front(); return rule;
                case LEFT_BRACE: {
                    rule.block = consumeSimpleBlock();
                    return rule;
                }
                default: {
                    rule.prelude.emplace_back(consumeComponentValue());
                }
            }
        }
        else {
            auto block = peek<SimpleBlock>();
            if (block.has_value() && block -> open.type == LEFT_BRACE) {
                rule.block = *block;
                values.pop_front();
                return rule;
            }
            else {
                rule.prelude.emplace_back(consumeComponentValue());
            }
        }
    }
    return rule;
}

Function BaseParser::consumeFunction() {
    Function f(consume(FUNCTION, "Expected function"));
    while (auto t = peek<Token>()) {
        switch (t -> type) {
            case T_EOF: case RIGHT_PAREN: {
                values.pop_front();
                return f;
            }
            default: {
                f.value.emplace_back(consumeComponentValue());
                break;
            }
        }
    }
    return f;
}

QualifiedRule BaseParser::consumeQualifiedRule() {
    QualifiedRule rule;
    while (!values.empty()) {
        if (auto t = peek<Token>()) {
            switch (t -> type) {
                case T_EOF: SYNTAX_ERROR("Qualified rule was not closed. Reached end of file.", nullopt);
                case LEFT_BRACE: {
                    rule.block = consumeSimpleBlock();
                    return rule;
                }
                case DELIM: {
                    if (t -> lexeme[0] == '$') {
                        return rule;
                    }
                }
                default: {
                    rule.prelude.emplace_back(consumeComponentValue());
                }
            }
        }
        else {
            auto block = peek<SimpleBlock>();
            if (block.has_value() && block -> open.type == LEFT_BRACE) {
                rule.block = *block;
                values.pop_front();
                return rule;
            }
            rule.prelude.emplace_back(consumeComponentValue());
        }
    }
    return rule;
}

SimpleBlock BaseParser::consumeSimpleBlock() {
    SimpleBlock block(consume<Token>());
    TokenType m = mirror(block.open.type);
    while (auto t = peek<Token>()) {
        if (t -> type == m) {
            block.close = *t;
            values.pop_front();
            return block;
        }
        else if (t -> type == T_EOF) {
            return block;
        }
        else {
            block.value.emplace_back(consumeComponentValue());
        }
    }
    return block;
}

#pragma endregion

#pragma region Selector Parser

COMPLEX_SELECTOR_LIST SelectorParser::parse() {
    IGNORE_WHITESPACE;
    COMPLEX_SELECTOR_LIST list;
    while (!values.empty()) {
        list.emplace_back(consumeComplexSelector());
        if (check(COMMA)) {
            values.pop_front();
            IGNORE_WHITESPACE;
        }
    }
    return list;
}

COMPLEX_SELECTOR SelectorParser::consumeComplexSelector() {
    IGNORE_WHITESPACE;
    COMPLEX_SELECTOR selectors = { COMPLEX_SELECTOR_PAIR({ }, consumeCompoundSelector()) };
    IGNORE_WHITESPACE;
    while (!values.empty() && !check(T_EOF) && !check(COMMA)) {
        COMBINATOR comb = consumeCombinator();
        IGNORE_WHITESPACE;
        selectors.emplace_back(comb, consumeCompoundSelector());
        IGNORE_WHITESPACE;
    }
    return selectors;
}

COMBINATOR SelectorParser::consumeCombinator() {
    auto tok = peek<Token>();
    switch (tok -> lexeme[0]) {
        case '>':
        case '+':
        case '~': {
            auto temp = std::move(*tok);
            values.pop_front();
            return temp;
        }
        case '|': {
            auto t1 = *tok;
            values.pop_front();
            Token t2 = consume(DELIM, "Expected DELIM");
            if (t2.lexeme[0] != '|') {
                SYNTAX_ERROR("Expected second |", t2);
            }
            return pair<Token, Token>(t1, t2);
        }
        default: {
            return { };
        }
    }
}

CompoundSelector SelectorParser::consumeCompoundSelector() {
    optional<TypeSelector> type = nullopt;
    if (check(IDENT) || check('*')) {
        type = consumeTypeSelector();
    }
    vector<SUBCLASS_SELECTOR> subclasses;
    bool consuming = true;
    while (consuming && !values.empty()) {
        if (auto t = peek<Token>()) {
            switch (t -> type) {
                case T_EOF: values.pop_front(); break;
                case DELIM: {
                    if (t -> lexeme[0] != '.') {
                        consuming = false;
                        break;
                    }
                    subclasses.emplace_back(consumeSubclassSelector());
                    break;
                }
                case COLON: {
                    bool isColon = check(COLON, 1);
                    if (isColon) {
                        consuming = false;
                        break;
                    }
                    subclasses.emplace_back(consumeSubclassSelector());
                    break;
                }
                case HASH:
                case LEFT_BRACKET: {
                    subclasses.emplace_back(consumeSubclassSelector());
                    break;
                }
                default: {
                    consuming = false;
                    break;
                }
            }
        }
        else {
            auto block = peek<SimpleBlock>();
            if (block.has_value() && block -> open.type == LEFT_BRACKET) {
                subclasses.emplace_back(consumeSubclassSelector());
            }
        }
    }
    vector<PSEUDO_SELECTOR_PAIR> pseudos;
    while (check(COLON)) {
        if (check(COLON, 1)) {
            PseudoElementSelector el = consumePseudoElementSelector();
            vector<PseudoClassSelector> classes;
            while (check(COLON)) {
                if (!check(COLON, 1)) {
                    classes.emplace_back(consumePseudoClassSelector());
                }
                else {
                    break;
                }
            }
            pseudos.emplace_back(el, classes);
        }
        else {
            break;
        }
    }
    if (!type.has_value() && subclasses.empty() && pseudos.empty()) {
        SYNTAX_ERROR("A compound selector requires at least one value. If there is a value at this position, it is most likely invalid.", peek<Token>());
    }
    return CompoundSelector(type, subclasses, pseudos);
}

AttrMatcher SelectorParser::consumeAttrMatcher() {
    auto t = consume(DELIM, "Expected delim");
    switch (t.lexeme[0]) {
        case '~':
        case '|':
        case '^':
        case '$':
        case '*': break;
        case '=': return { nullopt, t };
        default: {
            SYNTAX_ERROR("Expected ~, |, ^, $, *, or =", t);
        }
    }
    auto eq = consume(DELIM, "Expected =");
    if (eq.lexeme[0] == '=') {
        return { t, eq };
    }
    SYNTAX_ERROR("Expected =", eq);
}

NsPrefix SelectorParser::consumeNsPrefix() {
    auto t = consume<Token>();
    if (t.type == IDENT || (t.type == DELIM && t.lexeme[0] == '*')) {
        Token bar = consume(DELIM, "Expected delim");
        if (bar.lexeme[0] == '|') {
            return { t, bar };
        }
        SYNTAX_ERROR("Expected |", bar);
    }
    else if (t.type == DELIM && t.lexeme[0] == '|') {
        return { nullopt, t };
    }
    else {
        SYNTAX_ERROR("Expected |", t);
    }
}

WqName SelectorParser::consumeWqName() {
    auto t = peek<Token>();
    if (t -> type == IDENT) {
        if (check('|', 1)) {
            return { consumeNsPrefix(), consume(IDENT, "Expected identifier") };
        }
        auto temp = std::move(*t);
        values.pop_front();
        return { nullopt, temp };
    }
    else if (t -> type == DELIM && t -> lexeme[0] == '*') {
        return { consumeNsPrefix(), consume(IDENT, "Expected identifier") };
    }
    else {
        SYNTAX_ERROR("Expected NsPrefix or identifier", t);
    }
}

AttributeSelector SelectorParser::consumeAttributeSelector() {
    if (auto sb = peek<SimpleBlock>()) {
        sb -> value.insert(sb -> value.begin(), sb -> open);
        sb -> value.emplace_back(Token(RIGHT_BRACKET, L"]"));
        return SelectorParser(sb -> value).consumeAttributeSelector();
    }
    else {
        Token open = consume(LEFT_BRACKET, "Expected [");
        WqName name = consumeWqName();
        if (check(RIGHT_BRACKET)) {
            return {open, name, consume<Token>()};
        }
        else {
            AttrMatcher matcher = consumeAttrMatcher();
            auto tok = consume<Token>();
            if (tok.type != STRING && tok.type != IDENT) {
                SYNTAX_ERROR("Expected string or ident", tok);
            }
            optional<Token> mod;
            if (check('i') || check('I')) {
                mod = consume<Token>();
            }
            return {open, name, matcher, tok, mod, consume(RIGHT_BRACKET, "Expected closing bracket")};
        }
    }
}

TypeSelector SelectorParser::consumeTypeSelector() {
    auto t = peek<Token>();
    if (t -> type == DELIM && t -> lexeme[0] == '*') {
        if (check('|', 1)) {
            NsPrefix prefix = consumeNsPrefix();
            if (check('*')) {
                return { prefix, consume<Token>() };
            }
            SYNTAX_ERROR("Expected *", peek<Token>());
        }
        else {
            return { nullopt, consume<Token>() };
        }
    }
    else if (t -> type == IDENT) {
        if (check('|', 1)) {
            NsPrefix prefix = consumeNsPrefix();
            auto next = consume<Token>();
            if (next.type == DELIM && next.lexeme[0] == '*') {
                return { prefix, next };
            }
            else if (next.type == IDENT) {
                return { WqName(prefix, next) };
            }
        }
        else {
            return { consumeWqName() };
        }
    }
    SYNTAX_ERROR("Expected WqName or [NsPrefix? '*']", t);
}

ClassSelector SelectorParser::consumeClassSelector() {
    auto t = consume(DELIM, "Expected .");
    if (t.lexeme[0] == '.') {
        return { t, consume(IDENT, "Expected identifier") };
    }
    SYNTAX_ERROR("Expected .", t);
}

SUBCLASS_SELECTOR SelectorParser::consumeSubclassSelector() {
    if (check<SimpleBlock>()) {
        return consumeAttributeSelector();
    }
    else if (auto t = peek<Token>()) {
        switch (t -> type) {
            case HASH: {
                auto temp = std::move(*t);
                values.pop_front();
                return temp;
            }
            case DELIM: {
                if (t -> lexeme[0] == '.') {
                    return consumeClassSelector();
                }
                break;
            }
            case LEFT_BRACKET: return consumeAttributeSelector();
            case COLON: return consumePseudoClassSelector();
            default: break;
        }
    }
    return { };
}

vector<COMPONENT_VALUE> SelectorParser::consumeDeclarationValue(bool any) {
    vector<COMPONENT_VALUE> val;
    std::deque<TokenType> opening;
    while (true) {
        if (auto t = peek<Token>()) {
            switch (t -> type) {
                case BAD_STRING: case BAD_URL:
                {
                    values.pop_front();
                    return val;
                }
                case SEMICOLON: {
                    if (!any) {
                        return val;
                    }
                    val.emplace_back(*t);
                    values.pop_front();
                    break;
                }
                case DELIM: {
                    if (t -> lexeme[0] == '!' && !any) {
                        return val;
                    }
                    val.emplace_back(*t);
                    values.pop_front();
                    break;
                }
                case LEFT_PAREN: case LEFT_BRACE: case LEFT_BRACKET:
                {
                    opening.emplace_back(t -> type);
                    val.emplace_back(*t);
                    values.pop_front();
                    break;
                }
                case RIGHT_PAREN: case RIGHT_BRACE: case RIGHT_BRACKET:
                {
                    TokenType m = mirror(opening.back());
                    opening.pop_back();
                    if (t -> type != m) {
                        return val;
                    }
                    val.emplace_back(*t);
                    values.pop_front();
                    break;
                }
                default: {
                    val.emplace_back(*t);
                    values.pop_front();
                    break;
                }
            }
        }
        else {
            val.emplace_back(consume());
        }
    }
}

PseudoClassSelector SelectorParser::consumePseudoClassSelector() {
    auto colon = consume(COLON, "Expected colon");
    if (auto t = peek<Token>()) {
        if (t -> type == IDENT) {
            return {colon, *t};
        }
        else if (t -> type == FUNCTION) {
            return {colon, *t, consumeDeclarationValue(true), consume(RIGHT_PAREN, "Expected closing parenthesis")};
        }
    }
    else if (auto f = peek<Function>()) {
        return {colon, f -> name, f -> value, Token(RIGHT_PAREN, L")")};
    }
    SYNTAX_ERROR("Expected identifier or function", peek<Token>());
}

PseudoElementSelector SelectorParser::consumePseudoElementSelector() {
    Token colon = consume(COLON, "Expected :");
    return { colon, consumePseudoClassSelector() };
}

RelativeSelector SelectorParser::consumeRelativeSelector() {
    COMBINATOR comb = consumeCombinator();
    return { comb, consumeComplexSelector() };
}

SIMPLE_SELECTOR SelectorParser::consumeSimpleSelector() {
    SUBCLASS_SELECTOR subclass = consumeSubclassSelector();
    if (subclass.index() > 0) {
        return subclass;
    }
    else {
        return consumeTypeSelector();
    }
}

#pragma endregion

#pragma region Style Block Parser

STYLE_BLOCK StyleBlockParser::parse() {
    STYLE_BLOCK block;
    while (auto t = peek<Token>()) {
        switch (t -> type) {
            case WHITESPACE: case SEMICOLON: values.pop_front(); break;
            case T_EOF: {
                values.pop_front();
                return block;
            }
            case AT_KEYWORD: {
                block.emplace_back(consumeAtRule());
                break;
            }
            case IDENT: {
                vector<COMPONENT_VALUE> temp = { std::move(*t) };
                values.pop_front();
                while (!values.empty()) {
                    auto tok = peek<Token>();
                    if (tok && tok -> type == SEMICOLON) {
                        values.pop_front();
                        break;
                    }
                    temp.emplace_back(consumeComponentValue());
                }
                block.emplace_back(DeclarationParser(temp).parse());
                break;
            }
            case DELIM: {
                if (t -> lexeme[0] == '&') {
                    values.pop_front();
                    QualifiedRule rule = consumeQualifiedRule();
                    if (rule.block) {
                        block.emplace_back(StyleRule(SelectorParser(rule.prelude).parse(), StyleBlockParser(rule.block -> value).parse()));
                    }
                    else {
                        block.emplace_back(StyleRule(SelectorParser(rule.prelude).parse()));
                    }
                }
                break;
            }
            default: {
                while (!values.empty()) {
                    auto tok = peek<Token>();
                    if (tok && tok -> type == SEMICOLON) {
                        values.pop_front();
                        break;
                    }
                    consumeComponentValue();
                }
                break;
            }
        }
    }
    return block;
}

#pragma endregion

#pragma region Declaration Parser

Declaration DeclarationParser::parse() {
    Token name = consume(IDENT, "Expected identifier"); IGNORE_WHITESPACE;
    Token colon = consume(COLON, "Expected colon"); IGNORE_WHITESPACE;
    Declaration dec(name, colon);
    while (!values.empty()) {
        dec.value.emplace_back(consumeComponentValue());
    }
    if (dec.value.size() > 1) {
        COMPONENT_VALUE cvSlast = dec.value[dec.value.size() - 2];
        COMPONENT_VALUE cvLast = dec.value.back();
        if (std::holds_alternative<Token>(cvSlast) && std::holds_alternative<Token>(cvLast)) {
            Token tSlast = std::get<Token>(cvSlast);
            Token tLast = std::get<Token>(cvLast);
            if (tSlast.type == DELIM && tSlast.lexeme == L"!" && tLast.type == IDENT && wstrcompi(tLast.lexeme, L"important")) {
                dec.value.pop_back();
                dec.value.pop_back();
            }
        }
    }
    return dec;
}

#pragma endregion
