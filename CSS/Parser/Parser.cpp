// Eliminate as many check calls as possible for performance

#include "Macros.hpp"
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

/**
 * @brief Consumes the next SyntaxNode of type T
 * 
 * @tparam T The more specific type of the SyntaxNode (e.g Token, SimpleBlock, etc.)
 * @return SyntaxNode of type T on success
 * @return Throws an error on failure
 */

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
            case DELIM: {
                if (t -> lexeme[0] == '$') {
                    auto var = consumeVariable();
                    if (auto v = std::get_if<Variable>(&var)) {
                        list.emplace_back(*v);
                    }
                    else {
                        list.emplace_back(std::get<VariableDeclaration>(var));
                    }
                    break;
                }
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
            case DELIM: {
                if (t -> lexeme[0] == '$') {
                    auto var = consumeVariable();
                    if (auto v = std::get_if<Variable>(&var)) {
                        return *v;
                    }
                    else {
                        return std::get<VariableDeclaration>(var);
                    }
                    break;
                }
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

vector<COMPONENT_VALUE> BaseParser::consumeVariableValue() {
    IGNORE_WHITESPACE;
    vector<COMPONENT_VALUE> val;
    while (idx < values.size()) {
        if (auto t = peek<Token>()) {
            if ((t -> type == WHITESPACE && t -> lexeme.find('\n') != string::npos) || t -> type == SEMICOLON) {
                SKIP;
                break;
            }
        }
        COMPONENT_VALUE val = consumeComponentValue();
        if (auto block = std::get_if<SimpleBlock>(&val)) {
            if (std::holds_alternative<QualifiedRule>(values.back())) {
                values.pop_back();
                // TODO
            }
        }
        val.emplace_back(val);
    }
    return val;
}

variant<Variable, VariableDeclaration> BaseParser::consumeVariable() {
    Token dollar = consume(DELIM, "Expected delim");
    if (dollar.lexeme[0] != '$') SYNTAX_ERROR("Expected $", dollar);
    Variable var(dollar, consume(IDENT, "Expected identifier"));
    IGNORE_WHITESPACE;
    if (check('=')) {
        auto eq = consume<Token>();
        return VariableDeclaration(var, eq, consumeVariableValue());
    }
    else {
        return var;
    }
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
    optional<TypeSelector> type = nullopt;
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
            PseudoElementSelector el = consumePseudoElementSelector();
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

AttrMatcher SelectorParser::consumeAttrMatcher() {
    auto t = consume(DELIM, "Expected delim");
    switch (t.lexeme[0]) {
        case '~':
        case '|':
        case '^':
        case '$':
        case '*': break;
        case '=': return AttrMatcher(nullopt, t);
        default: {
            SYNTAX_ERROR("Expected ~, |, ^, $, *, or =", t);
        }
    }
    auto eq = consume(DELIM, "Expected =");
    if (eq.lexeme[0] == '=') {
        return AttrMatcher(t, eq);
    }
    SYNTAX_ERROR("Expected =", eq);
}

NsPrefix SelectorParser::consumeNsPrefix() {
    auto t = consume<Token>();
    if (t.type == IDENT || (t.type == DELIM && t.lexeme[0] == '*')) {
        Token bar = consume(DELIM, "Expected delim");
        if (bar.lexeme[0] == '|') {
            return NsPrefix(t, bar);
        }
        SYNTAX_ERROR("Expected |", bar);
    }
    else if (t.type == DELIM && t.lexeme[0] == '|') {
        return NsPrefix(nullopt, t);
    }
    else {
        SYNTAX_ERROR("Expected |", t);
    }
}

WqName SelectorParser::consumeWqName() {
    auto t = consume<Token>();
    if (t.type == IDENT) {
        if (check('|')) {
            RECONSUME;
            return WqName(consumeNsPrefix(), consume(IDENT, "Expected identifier"));
        }
        return WqName(nullopt, t);
    }
    else if (t.type == DELIM && t.lexeme[0] == '*') {
        RECONSUME;
        return WqName(consumeNsPrefix(), consume(IDENT, "Expected identifier"));
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
    auto t = consume<Token>();
    if (t.type == DELIM && t.lexeme[0] == '*') {
        if (check('|')) {
            RECONSUME;
            NsPrefix prefix = consumeNsPrefix();
            if (check('*')) {
                return TypeSelector(prefix, consume(DELIM, "Expected delim"));
            }
            SYNTAX_ERROR("Expected *", peek<Token>());
        }
        else {
            return TypeSelector(nullopt, t);
        }
    }
    else if (t.type == IDENT) {
        if (check('|')) {
            RECONSUME;
            NsPrefix prefix = consumeNsPrefix();
            Token next = consume<Token>();
            if (next.type == DELIM && next.lexeme[0] == '*') {
                return TypeSelector(prefix, next);
            }
            else if (next.type == IDENT) {
                return TypeSelector(WqName(prefix, next));
            }
        }
        else {
            RECONSUME;
            return TypeSelector(consumeWqName());
        }
    }
    SYNTAX_ERROR("Expected WqName or [NsPrefix? '*']", t);
}

ClassSelector SelectorParser::consumeClassSelector() {
    auto t = consume(DELIM, "Expected .");
    if (t.lexeme[0] == '.') {
        return ClassSelector(t, consume(IDENT, "Expected identifier"));
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

PseudoElementSelector SelectorParser::consumePseudoElementSelector() {
    Token colon = consume(COLON, "Expected :");
    return PseudoElementSelector(colon, consumePseudoClassSelector());
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