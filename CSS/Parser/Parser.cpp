// Eliminate as many check calls as possible for performance

#pragma region includes

#include "../Util/util.hpp"
#include "Grammar/Selector.hpp"
#include "Parser.hpp"
#include <deque>
#include <cctype>
#include <utility>
#include <iostream>

#include <chrono>
using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::milliseconds;

#pragma endregion

#pragma region Helpers
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

template<typename T>
T ComponentValueParser::consume() {
    if (auto val = peek<T>()) {
        idx++;
        return *val;
    }
    else {
        SYNTAX_ERROR(string("Called consume method with an invalid type. The specified type (") + string(typeid(T).name()).substr(1) + ") does not match the next token's type.", nullopt);
    }
}

Token ComponentValueParser::consume(TokenType type, const string& error) {
    auto t = consume<Token>();
    if (t.type == type) {
        return t;
    }
    SYNTAX_ERROR(error, t);
}

template<>
optional<COMPONENT_VALUE> ComponentValueParser::peek() {
    return idx >= 0 && idx < values.size() ? optional<COMPONENT_VALUE>(values[idx]) : nullopt;
}

template<typename T>
optional<T> ComponentValueParser::peek() {
    if (idx >= 0 && idx < values.size()) {
        if (const T* val = std::get_if<T>(&values[idx])) {
            return *val;
        }
    }
    return nullopt;
}

template<typename T>
bool ComponentValueParser::check() {
    return peek<T>().has_value();
}

bool ComponentValueParser::check(TokenType type) {
    auto t = peek<Token>();
    return t.has_value() && t -> type == type;
}

bool ComponentValueParser::check(const wstring& lexeme) {
    auto t = peek<Token>();
    return t.has_value() && t -> lexeme == lexeme;
}

bool ComponentValueParser::check(const wchar_t& lexeme) {
    auto t = peek<Token>();
    return t.has_value() && t -> type == DELIM && t -> lexeme[0] == lexeme;
}

#pragma endregion

#pragma region Base Parser

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
            case WHITESPACE: SKIP; break;
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
                SKIP;
                return *t;
            }
        }
    }
    return consume<COMPONENT_VALUE>();
}

AtRule BaseParser::consumeAtRule() {
    Token at = consume(AT_KEYWORD, "Expected AT_KEYWORD");
    AtRule rule(at);
    while (idx < values.size()) {
        if (auto t = peek<Token>()) {
            switch (t -> type) {
                case T_EOF: case SEMICOLON: SKIP; return rule;
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
                SKIP;
                rule.block = *block;
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
                SKIP;
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
    while (idx < values.size()) {
        if (auto t = peek<Token>()) {
            switch (t -> type) {
                case T_EOF: SYNTAX_ERROR("Qualified rule was not closed. Reached end of file.", nullopt);
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
                SKIP;
                rule.block = *block;
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
            SKIP;
            block.close = *t;
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
    while (idx < values.size()) {
        list.emplace_back(consumeComplexSelector());
        if (check(COMMA)) {
            SKIP;
            IGNORE_WHITESPACE;
        }
    }
    return list;
}

COMPLEX_SELECTOR SelectorParser::consumeComplexSelector() {
    IGNORE_WHITESPACE;
    COMPLEX_SELECTOR selectors = { COMPLEX_SELECTOR_PAIR({ }, consumeCompoundSelector()) };
    IGNORE_WHITESPACE;
    while (NOT_EOF && !check(COMMA)) {
        COMBINATOR comb = consumeCombinator();
        IGNORE_WHITESPACE;
        selectors.emplace_back(comb, consumeCompoundSelector());
        IGNORE_WHITESPACE;
    }
    return selectors;
}

COMBINATOR SelectorParser::consumeCombinator() {
    auto tok = consume<Token>();
    switch (tok.lexeme[0]) {
        case '>':
        case '+':
        case '~': return tok;
        case '|': {
            Token t2 = consume(DELIM, "Expected DELIM");
            if (t2.lexeme[0] != '|') {
                SYNTAX_ERROR("Expected second |", t2);
            }
            return pair<Token, Token>(tok, t2);
        }
        default: {
            RECONSUME;
            return { };
        }
    }
}

CompoundSelector SelectorParser::consumeCompoundSelector() {
    optional<TYPE_SELECTOR> type = nullopt;
    if (check(IDENT) || check('*')) {
        type = consumeTypeSelector();
    }
    vector<SUBCLASS_SELECTOR> subclasses;
    bool consuming = true;
    while (consuming && idx < values.size()) {
        if (auto t = peek<Token>()) {
            switch (t -> type) {
                case T_EOF: SKIP; break;
                case DELIM: {
                    if (t -> lexeme[0] != '.') {
                        consuming = false;
                        break;
                    }
                    subclasses.emplace_back(consumeSubclassSelector());
                    break;
                }
                case COLON: {
                    SKIP;
                    bool isColon = check(COLON);
                    RECONSUME;
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
        SKIP;
        if (check(COLON)) {
            RECONSUME;
            PSEUDO_ELEMENT_SELECTOR el = consumePseudoElementSelector();
            vector<PseudoClassSelector> classes;
            while (check(COLON)) {
                SKIP;
                if (!check(COLON)) {
                    RECONSUME;
                    classes.emplace_back(consumePseudoClassSelector());
                }
                else {
                    RECONSUME;
                    break;
                }
            }
            pseudos.emplace_back(el, classes);
        }
        else {
            RECONSUME;
            break;
        }
    }
    if (!type.has_value() && subclasses.empty() && pseudos.empty()) {
        SYNTAX_ERROR("A compound selector requires at least one value. If there is a value at this position, it is most likely invalid.", peek<Token>());
    }
    return CompoundSelector(type, subclasses, pseudos);
}

ATTR_MATCHER SelectorParser::consumeAttrMatcher() {
    auto t = consume(DELIM, "Expected delim");
    switch (t.lexeme[0]) {
        case '~':
        case '|':
        case '^':
        case '$':
        case '*': break;
        case '=': return ATTR_MATCHER(nullopt, t);
        default: {
            SYNTAX_ERROR("Expected ~, |, ^, $, *, or =", t);
        }
    }
    auto eq = consume(DELIM, "Expected =");
    if (eq.lexeme[0] == '=') {
        return ATTR_MATCHER(t, eq);
    }
    SYNTAX_ERROR("Expected =", eq);
}

NS_PREFIX SelectorParser::consumeNsPrefix() {
    auto t = consume<Token>();
    if (t.type == IDENT || (t.type == DELIM && t.lexeme[0] == '*')) {
        Token bar = consume(DELIM, "Expected delim");
        if (bar.lexeme[0] == '|') {
            return NS_PREFIX(t, bar);
        }
        SYNTAX_ERROR("Expected |", bar);
    }
    else if (t.type == DELIM && t.lexeme[0] == '|') {
        return NS_PREFIX(nullopt, t);
    }
    else {
        SYNTAX_ERROR("Expected |", t);
    }
}

WQ_NAME SelectorParser::consumeWqName() {
    auto t = consume<Token>();
    if (t.type == IDENT) {
        if (check('|')) {
            RECONSUME;
            return WQ_NAME(consumeNsPrefix(), consume(IDENT, "Expected identifier"));
        }
        return WQ_NAME(nullopt, t);
    }
    else if (t.type == DELIM && t.lexeme[0] == '*') {
        RECONSUME;
        return WQ_NAME(consumeNsPrefix(), consume(IDENT, "Expected identifier"));
    }
    else {
        SYNTAX_ERROR("Expected NS_PREFIX or identifier", t);
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
        WQ_NAME name = consumeWqName();
        if (check(RIGHT_BRACKET)) {
            return {open, name, consume<Token>()};
        }
        else {
            ATTR_MATCHER matcher = consumeAttrMatcher();
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

TYPE_SELECTOR SelectorParser::consumeTypeSelector() {
    auto t = consume<Token>();
    if (t.type == DELIM && t.lexeme[0] == '*') {
        if (check('|')) {
            RECONSUME;
            NS_PREFIX prefix = consumeNsPrefix();
            if (check('*')) {
                return TYPE_SELECTOR(prefix, consume(DELIM, "Expected delim"));
            }
            SYNTAX_ERROR("Expected *", peek<Token>());
        }
        else {
            return TYPE_SELECTOR(nullopt, t);
        }
    }
    else if (t.type == IDENT) {
        if (check('|')) {
            RECONSUME;
            NS_PREFIX prefix = consumeNsPrefix();
            Token next = consume<Token>();
            if (next.type == DELIM && next.lexeme[0] == '*') {
                return TYPE_SELECTOR(prefix, next);
            }
            else if (next.type == IDENT) {
                return WQ_NAME(prefix, next);
            }
        }
        else {
            RECONSUME;
            return consumeWqName();
        }
    }
    SYNTAX_ERROR("Expected WQ_NAME or [NS_PREFIX? '*']", t);
}

CLASS_SELECTOR SelectorParser::consumeClassSelector() {
    auto t = consume(DELIM, "Expected .");
    if (t.lexeme[0] == '.') {
        return CLASS_SELECTOR(t, consume(IDENT, "Expected identifier"));
    }
    SYNTAX_ERROR("Expected .", t);
}

SUBCLASS_SELECTOR SelectorParser::consumeSubclassSelector() {
    if (check<SimpleBlock>()) {
        return consumeAttributeSelector();
    }
    else if (auto t = peek<Token>()) {
        switch (t -> type) {
            case HASH: SKIP; return *t;
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
                    SKIP;
                    return val;
                }
                case SEMICOLON: {
                    if (!any) {
                        return val;
                    }
                    SKIP;
                    val.emplace_back(*t);
                    break;
                }
                case DELIM: {
                    if (t -> lexeme[0] == '!' && !any) {
                        return val;
                    }
                    SKIP;
                    val.emplace_back(*t);
                    break;
                }
                case LEFT_PAREN: case LEFT_BRACE: case LEFT_BRACKET:
                {
                    SKIP;
                    opening.emplace_back(t -> type);
                    val.emplace_back(*t);
                    break;
                }
                case RIGHT_PAREN: case RIGHT_BRACE: case RIGHT_BRACKET:
                {
                    TokenType m = mirror(opening.back());
                    opening.pop_back();
                    if (t -> type != m) {
                        return val;
                    }
                    SKIP;
                    val.emplace_back(*t);
                    break;
                }
                default: {
                    SKIP;
                    val.emplace_back(*t);
                    break;
                }
            }
        }
        else {
            val.emplace_back(consume<COMPONENT_VALUE>());
        }
    }
}

PseudoClassSelector SelectorParser::consumePseudoClassSelector() {
    auto colon = consume(COLON, "Expected colon");
    if (auto t = peek<Token>()) {
        SKIP;
        if (t -> type == IDENT) {
            return {colon, *t};
        }
        else if (t -> type == FUNCTION) {
            return {colon, *t, consumeDeclarationValue(true), consume(RIGHT_PAREN, "Expected closing parenthesis")};
        }
    }
    else if (auto f = peek<Function>()) {
        SKIP;
        return {colon, f -> name, f -> value, Token(RIGHT_PAREN, L")")};
    }
    SYNTAX_ERROR("Expected identifier or function", peek<Token>());
}

PSEUDO_ELEMENT_SELECTOR SelectorParser::consumePseudoElementSelector() {
    Token colon = consume(COLON, "Expected :");
    return PSEUDO_ELEMENT_SELECTOR(colon, consumePseudoClassSelector());
}

RelativeSelector SelectorParser::consumeRelativeSelector() {
    COMBINATOR comb = consumeCombinator();
    return {comb, consumeComplexSelector()};
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
            case WHITESPACE: case SEMICOLON: SKIP; break;
            case T_EOF: {
                SKIP;
                return block;
            }
            case AT_KEYWORD: {
                block.emplace_back(consumeAtRule());
                break;
            }
            case IDENT: {
                SKIP;
                vector<COMPONENT_VALUE> temp = { *t };
                while (idx < values.size()) {
                    auto t = peek<Token>();
                    if (t -> type == SEMICOLON) {
                        SKIP;
                        break;
                    }
                    temp.emplace_back(consumeComponentValue());
                }
                block.emplace_back(DeclarationParser(temp).parse());
                break;
            }
            case DELIM: {
                if (t -> lexeme[0] == '&') {
                    SKIP;
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
                while (idx < values.size()) {
                    auto t = peek<Token>();
                    if (t -> type == SEMICOLON) {
                        SKIP;
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
    while (idx < values.size()) {
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