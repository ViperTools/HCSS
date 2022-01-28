#include "../Util/util.hpp"
#include "Parser.hpp"
#include <deque>
#include <cctype>
#include <utility>
#include <iostream>

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
    if (check<T>()) {
        T val = *peek<T>();
        idx++;
        return val;
    }
    else {
        SYNTAX_ERROR("Called consume method with an invalid type. The specified type does not match the next token.", nullopt);
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
        COMPONENT_VALUE val = values[idx];
        if (std::holds_alternative<T>(val)) {
            return std::get<T>(val);
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

#pragma endregion

#pragma region Base Parser

vector<SYNTAX_NODE> BaseParser::parse() {
    rules = consumeRulesList();
    top = false;
    for (auto & i : rules) {
        SYNTAX_NODE node = i;
        if (std::holds_alternative<QualifiedRule>(node)) {
            QualifiedRule rule = std::get<QualifiedRule>(node);
            COMPLEX_SELECTOR_LIST selectors = SelectorParser(rule.prelude).parse();
            if (rule.block.has_value()) {
                STYLE_BLOCK block = StyleBlockParser(rule.block.value().value).parse();
                i = StyleRule(selectors, block);
            }
            else {
                i = StyleRule(selectors);
            }
        }
    }
    return rules;
}

vector<SYNTAX_NODE> BaseParser::consumeRulesList() {
    vector<SYNTAX_NODE> list;
    while (NOT_EOF) {
        auto t = consume<Token>();
        switch (t.type) {
            case WHITESPACE: break;
            case T_EOF: {
                return list;
            }
            case CDO:
            case CDC: {
                if (!top) {
                    list.emplace_back(consumeQualifiedRule());
                }
                break;
            }
            case AT_KEYWORD: {
                RECONSUME;
                list.emplace_back(consumeAtRule());
                break;
            }
            default: {
                RECONSUME;
                list.emplace_back(consumeQualifiedRule());
            }
        }
    }
    return list;
}

COMPONENT_VALUE BaseParser::consumeComponentValue() {
    if (check<Token>()) {
        auto t = consume<Token>();
        switch (t.type) {
            case LEFT_BRACE: case LEFT_BRACKET: case LEFT_PAREN: {
                RECONSUME;
                return consumeSimpleBlock();
            }
            case FUNCTION: {
                RECONSUME;
                return consumeFunction();
            }
            default: {
                return t;
            }
        }
    }
    return consume<COMPONENT_VALUE>();
}

AtRule BaseParser::consumeAtRule() {
    Token at = consume(AT_KEYWORD, "Expected AT_KEYWORD");
    AtRule rule(at);
    while (NOT_EOF) {
        if (check<Token>()) {
            auto t = consume<Token>();
            switch (t.type) {
                case T_EOF:
                case SEMICOLON: return rule;
                case LEFT_BRACE: {
                    RECONSUME;
                    rule.block = consumeSimpleBlock();
                    return rule;
                }
                default: {
                    RECONSUME;
                    rule.prelude.emplace_back(consumeComponentValue());
                }
            }
        }
        else if (check<SimpleBlock>() && peek<SimpleBlock>() -> open.type == LEFT_BRACE) {
            rule.block = consume<SimpleBlock>();
            return rule;
        }
        else {
            RECONSUME;
            rule.prelude.emplace_back(consumeComponentValue());
        }
    }
    return rule;
}

Function BaseParser::consumeFunction() {
    Token name = consume(FUNCTION, "Expected function");
    Function f(name);
    while (NOT_EOF) {
        auto t = consume<Token>();
        switch (t.type) {
            case T_EOF:
            case RIGHT_PAREN: {
                return f;
            }
            default: {
                RECONSUME;
                f.value.emplace_back(consumeComponentValue());
            }
        }
    }
    return f;
}

QualifiedRule BaseParser::consumeQualifiedRule() {
    QualifiedRule rule;
    while (NOT_EOF) {
        if (check<Token>()) {
            auto t = consume<Token>();
            switch (t.type) {
                case T_EOF: SYNTAX_ERROR("Qualified rule was not closed. Reached end of file.", nullopt);
                case LEFT_BRACE: {
                    RECONSUME;
                    rule.block = consumeSimpleBlock();
                    return rule;
                }
                default: {
                    RECONSUME;
                    rule.prelude.emplace_back(consumeComponentValue());
                }
            }
        }
        else if (check<SimpleBlock>() && peek<SimpleBlock>() -> open.type == LEFT_BRACE) {
            rule.block = consume<SimpleBlock>();
            return rule;
        }
        else {
            RECONSUME;
            rule.prelude.emplace_back(consumeComponentValue());
        }
    }
    return rule;
}

SimpleBlock BaseParser::consumeSimpleBlock() {
    auto open = consume<Token>();
    SimpleBlock block(open);
    TokenType m = mirror(open.type);
    vector<COMPONENT_VALUE> components;
    while (NOT_EOF) {
        auto t = consume<Token>();
        if (t.type == T_EOF || t.type == m) {
            return block;
        }
        else {
            RECONSUME;
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
    while (NOT_EOF) {
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
            if (t2.lexeme != L"|") {
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
    if (check(IDENT) || peek<Token>() -> lexeme == L"*") {
        type = consumeTypeSelector();
    }
    vector<SUBCLASS_SELECTOR> subclasses;
    bool consuming = true;
    while (consuming && NOT_EOF) {
        auto t = peek<Token>();
        switch (t -> type) {
            case DELIM: {
                if (t -> lexeme != L".") {
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
    vector<PseudoSelectorPair> pseudos;
    while (check(COLON)) {
        auto colon = consume<Token>();
        if (check(COLON)) {
            RECONSUME;
            PSEUDO_ELEMENT_SELECTOR el = consumePseudoElementSelector();
            vector<PseudoClassSelector> classes;
            while (check(COLON)) {
                auto t = consume<Token>();
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
    Token t = consume(DELIM, "Expected delim");
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
    if (check(DELIM) && peek<Token>() -> lexeme == L"=") {
        return ATTR_MATCHER(t, consume<Token>());
    }
    SYNTAX_ERROR("Expected =", peek<Token>());
}

NS_PREFIX SelectorParser::consumeNsPrefix() {
    auto t = consume<Token>();
    if (t.type == IDENT || (t.type == DELIM && t.lexeme == L"*")) {
        Token bar = consume(DELIM, "Expected delim");
        if (bar.lexeme == L"|") {
            return NS_PREFIX(t, bar);
        }
        SYNTAX_ERROR("Expected |", bar);
    }
    else if (t.type == DELIM && t.lexeme == L"|") {
        return NS_PREFIX(nullopt, t);
    }
    else {
        SYNTAX_ERROR("Expected |", t);
    }
}

WQ_NAME SelectorParser::consumeWqName() {
    auto t = consume<Token>();
    if (t.type == IDENT) {
        if (check(DELIM) && peek<Token>() -> lexeme == L"|") {
            RECONSUME;
            return WQ_NAME(consumeNsPrefix(), consume(IDENT, "Expected identifier"));
        }
        return WQ_NAME(nullopt, t);
    }
    else if (t.lexeme == L"*") {
        RECONSUME;
        return WQ_NAME(consumeNsPrefix(), consume(IDENT, "Expected identifier"));
    }
    else {
        SYNTAX_ERROR("Expected NS_PREFIX or identifier", peek<Token>());
    }
}

// TODO Allow consuming a simple block
AttributeSelector SelectorParser::consumeAttributeSelector() {
    Token open = consume(LEFT_BRACKET, "Expected [");
    WQ_NAME name = consumeWqName();
    if (check(RIGHT_BRACKET)) {
        return {open, name, consume<Token>()};
    }
    ATTR_MATCHER matcher = consumeAttrMatcher();
    if (!check(STRING) && !check(IDENT)) {
        SYNTAX_ERROR("Expected string or ident", peek<Token>());
    }
    auto tok = consume<Token>();
    if (check(RIGHT_BRACKET)) {
        return {open, name, matcher, tok, nullopt, consume<Token>()};
    }
    if (check(DELIM) && tolower((peek<Token>() -> lexeme)[0]) == 'i') {
        auto mod = consume<Token>();
        if (check(RIGHT_BRACKET)) {
            return {open, name, matcher, tok, mod, consume<Token>()};
        }
        SYNTAX_ERROR("Expected closing bracket", peek<Token>());
    }
    SYNTAX_ERROR("Expected modifier or closing token", peek<Token>());
}

TYPE_SELECTOR SelectorParser::consumeTypeSelector() {
    auto t = consume<Token>();
    if (t.type == DELIM && t.lexeme == L"*") {
        if (check(L"|")) {
            RECONSUME;
            NS_PREFIX prefix = consumeNsPrefix();
            if (check(L"*")) {
                return TYPE_SELECTOR(prefix, consume(DELIM, "Expected delim"));
            }
            SYNTAX_ERROR("Expected *", peek<Token>());
        }
        else {
            return TYPE_SELECTOR(nullopt, t);
        }
    }
    else if (t.type == IDENT) {
        if (check(L"|")) {
            RECONSUME;
            NS_PREFIX prefix = consumeNsPrefix();
            if (check(L"*")) {
                return TYPE_SELECTOR(prefix, consume(DELIM, "Expected delim"));
            }
            else if (check(IDENT)) {
                return WQ_NAME(prefix, consume<Token>());
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
    if (check(DELIM) && peek<Token>() -> lexeme == L".") {
        auto dot = consume<Token>();
        if (check(IDENT)) {
            return CLASS_SELECTOR(dot, consume<Token>());
        }
        SYNTAX_ERROR("Expected identifier", dot);
    }
    SYNTAX_ERROR("Expected .", peek<Token>());
}

SUBCLASS_SELECTOR SelectorParser::consumeSubclassSelector() {
    if (check(HASH)) {
        return consume<Token>();
    }
    else if (check(DELIM) && peek<Token>() -> lexeme == L".") {
        return consumeClassSelector();
    }
    else if (check(LEFT_BRACKET)) {
        return consumeAttributeSelector();
    }
    else if (check(COLON)) {
        return consumePseudoClassSelector();
    }
    return { };
}

vector<COMPONENT_VALUE> SelectorParser::consumeDeclarationValue(bool any) {
    vector<COMPONENT_VALUE> val;
    while (true) {
        if (check<Token>()) {
            auto t = consume<Token>();
            std::deque<TokenType> opening;
            switch (t.type) {
                case BAD_STRING: case BAD_URL:
                {
                    return val;
                }
                case SEMICOLON: {
                    if (!any) {
                        RECONSUME;
                        return val;
                    }
                    val.emplace_back(t);
                    break;
                }
                case DELIM: {
                    if (t.lexeme == L"!" && !any) {
                        RECONSUME;
                        return val;
                    }
                    val.emplace_back(t);
                    break;
                }
                case LEFT_PAREN: case LEFT_BRACE: case LEFT_BRACKET:
                {
                    opening.emplace_back(t.type);
                    val.emplace_back(t);
                    break;
                }
                case RIGHT_PAREN: case RIGHT_BRACE: case RIGHT_BRACKET:
                {
                    TokenType m = mirror(opening.back());
                    opening.pop_back();
                    if (t.type != m) {
                        RECONSUME;
                        return val;
                    }
                    val.emplace_back(t);
                    break;
                }
                default: {
                    val.emplace_back(t);
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
    if (check(COLON)) {
        auto colon = consume<Token>();
        if (check(IDENT)) {
            return {colon, consume<Token>()};
        }
        else if (check(FUNCTION)) {
            auto func = consume<Token>();
            vector<COMPONENT_VALUE> any = consumeDeclarationValue(true);
            return {colon, func, any, consume(RIGHT_PAREN, "Expected closing parenthesis")};
        }
        else if (check<Function>()) {
            Function f = consume<Function>();
            return {colon, f.name, f.value};
        }
    }
    SYNTAX_ERROR("Expected colon or function", peek<Token>());
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
    vector<Declaration> decls;
    vector<RULE> rules;
    STYLE_BLOCK block;
    while (NOT_EOF) {
        auto t = consume<Token>();
        switch (t.type) {
            case WHITESPACE: case SEMICOLON: break;
            case T_EOF: {
                std::move(decls.begin(), decls.end(), block.begin());
                std::move(rules.begin(), rules.end(), block.end());
                return block;
            }
            case AT_KEYWORD: {
                RECONSUME;
                rules.emplace_back(consumeAtRule());
                break;
            }
            case IDENT: {
                vector<COMPONENT_VALUE> temp = { t };
                while (NOT_EOF && !check(SEMICOLON)) {
                    temp.emplace_back(consumeComponentValue());
                }
                DeclarationParser parser(temp);
                decls.emplace_back(parser.parse());
                break;
            }
            case DELIM: {
                if (t.lexeme == L"&") {
                    RECONSUME;
                    rules.emplace_back(consumeQualifiedRule());
                }
                break;
            }
            default: {
                RECONSUME;
                while (NOT_EOF && !check(SEMICOLON)) {
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
    Token name = consume(IDENT, "Expected identifier");
    IGNORE_WHITESPACE;
    Token colon = consume(COLON, "Expected colon");
    IGNORE_WHITESPACE;
    Declaration dec(name, colon);
    while (NOT_EOF) {
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