#include "../Util/util.hpp"
#include "Parser.hpp"
#include "Errors/SyntaxError.hpp"
#include <iostream>
#include <deque>
#include <cctype>
#include <utility>

#pragma region Helpers
TokenType mirror(TokenType type) {
    switch (type) {
        case LEFT_BRACE: return RIGHT_BRACE;
        case LEFT_BRACKET: return RIGHT_BRACKET;
        case LEFT_PAREN: return RIGHT_PAREN;
        default: throw SyntaxError(nullptr, "Expected {, [, or (");
    }
}

#pragma endregion

#pragma region Component Parser

template<typename T = COMPONENT_VALUE>
T ComponentValueParser::consume() {
    if (check<T>()) {
        return *peek<T>();
    }
    else {
        throw SyntaxError(nullptr, "Attempted to consume an invalid type.");
    }
}

Token ComponentValueParser::consume(TokenType type, const string& error) {
    Token t = consume<Token>();
    if (t.type == type) {
        return t;
    }
    throw SyntaxError(&t, error);
}

template<typename T = COMPONENT_VALUE>
T* ComponentValueParser::peek() {
    if (idx >= 0 && idx < values.size()) {
        COMPONENT_VALUE val = values[idx];
        if (std::holds_alternative<T>(val)) {
            return &std::get<T>(val);
        }
    }
    return nullptr;
}

template<typename T = COMPONENT_VALUE>
bool ComponentValueParser::check() {
    return peek<T>() != nullptr;
}

bool ComponentValueParser::check(TokenType type) {
    Token* t = peek<Token>();
    return t != nullptr && t -> type == type;
}

bool ComponentValueParser::check(const wstring& lexeme) {
    Token* t = peek<Token>();
    return t != nullptr && t -> lexeme == lexeme;
}

#pragma endregion

#pragma region Base Parser

vector<SYNTAX_NODE> BaseParser::parse() {
    rules = consumeRulesList();
    top = false;
    for (int i = 0; i < rules.size(); i++) {
        SYNTAX_NODE node = rules[i];
        if (std::holds_alternative<QualifiedRule>(node)) {
            QualifiedRule rule = std::get<QualifiedRule>(node);
            COMPLEX_SELECTOR_LIST selectors = SelectorParser(rule.components).parse();
            if (rule.block.has_value()) {
                STYLE_BLOCK block = StyleBlockParser(rule.block.value().components).parse();
                rules[i] = StyleRule(selectors, block);
            }
            else {
                rules[i] = StyleRule(selectors);
            }
        }
    }
    return rules;
}

vector<SYNTAX_NODE> BaseParser::consumeRulesList() {
    vector<SYNTAX_NODE> list;
    while (NOT_EOF) {
        Token t = consume<Token>();
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
    COMPONENT_VALUE v = consume<COMPONENT_VALUE>();
    if (check<Token>()) {
        Token t = consume<Token>();
        switch (t.type) {
            case LEFT_BRACE: case LEFT_BRACKET: case LEFT_PAREN: {
                return consumeSimpleBlock();
            }
            case FUNCTION: {
                return consumeFunction();
            }
            default: {
                return t;
            }
        }
    }
    return v;
}

AtRule BaseParser::consumeAtRule() {
    Token at = consume(AT_KEYWORD, "Expected AT_KEYWORD");
    AtRule rule(at);
    while (NOT_EOF) {
        Token t = consume<Token>();
        switch (t.type) {
            case T_EOF:
            case SEMICOLON: return rule;
            case LEFT_BRACE: {
                rule.block = consumeSimpleBlock();
                return rule;
            }
            default: {
                RECONSUME;
                rule.components.push_back(consumeComponentValue());
            }
        }
    }
    return rule;
}

Function BaseParser::consumeFunction() {
    RECONSUME;
    Token ident = consume(FUNCTION, "Expected function");
    Function f(ident);
    while (NOT_EOF) {
        Token t = consume<Token>();
        switch (t.type) {
            case T_EOF:
            case RIGHT_PAREN: {
                return f;
            }
            default: {
                RECONSUME;
                f.components.push_back(consumeComponentValue());
            }
        }
    }
    return f;
}

QualifiedRule BaseParser::consumeQualifiedRule() {
    QualifiedRule rule;
    while (NOT_EOF) {
        Token t = consume<Token>();
        switch (t.type) {
            case T_EOF: return rule; //NULL;
            case LEFT_BRACE: {
                rule.block = consumeSimpleBlock();
                return rule;
            }
            default: {
                RECONSUME;
                rule.components.push_back(consumeComponentValue());
            }
        }
    }
    return rule;
}

SimpleBlock BaseParser::consumeSimpleBlock() {
    RECONSUME;
    Token open = consume<Token>();
    SimpleBlock block(open);
    TokenType m = mirror(open.type);
    vector<COMPONENT_VALUE> components;
    while (NOT_EOF) {
        Token t = consume<Token>();
        if (t.type == T_EOF || t.type == m) {
            return block;
        }
        else {
            RECONSUME;
            block.components.push_back(consumeComponentValue());
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
        list.push_back(consumeComplexSelector());
        if (check(COMMA)) {
            SKIP;
            IGNORE_WHITESPACE;
        }
    }
    return list;
}

ComplexSelector SelectorParser::consumeComplexSelector() {
    vector<COMPLEX_SELECTOR_PAIR> selectors = {COMPLEX_SELECTOR_PAIR({ }, consumeCompoundSelector()) };
    IGNORE_WHITESPACE;
    while (NOT_EOF && !check(COMMA)) {
        COMBINATOR comb = consumeCombinator();
        IGNORE_WHITESPACE;
        selectors.emplace_back(comb, consumeCompoundSelector());
        IGNORE_WHITESPACE;
    }
    return ComplexSelector(vector<COMPLEX_SELECTOR_PAIR>());
}

COMBINATOR SelectorParser::consumeCombinator() {
    Token tok = consume<Token>();
    switch (tok.lexeme[0]) {
        case '>':
        case '+':
        case '~': return tok;
        case '|': {
            Token t2 = consume(DELIM, "Expected DELIM");
            if (t2.lexeme != L"|") {
                throw SyntaxError(&t2, "Expected second |");
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
        Token* t = peek<Token>();
        switch (t -> type) {
            case DELIM: {
                if (t -> lexeme != L".") {
                    consuming = false;
                    break;
                }
                subclasses.push_back(consumeSubclassSelector());
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
                subclasses.push_back(consumeSubclassSelector());
                break;
            }
            case HASH:
            case LEFT_BRACKET: {
                subclasses.push_back(consumeSubclassSelector());
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
        Token colon = consume<Token>();
        if (check(COLON)) {
            PSEUDO_ELEMENT_SELECTOR el = consumePseudoElementSelector();
            vector<PseudoClassSelector> classes;
            while (check(COLON)) {
                Token t = consume<Token>();
                if (!check(COLON)) {
                    RECONSUME;
                    classes.push_back(consumePseudoClassSelector());
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
        throw SyntaxError(peek<Token>(), "A compound selector requires at least one value. If there is a value at this position, it is most likely invalid.");
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
            throw SyntaxError(&t, "Expected ~, |, ^, $, *, or =");
        }
    }
    if (check(DELIM) && peek<Token>() -> lexeme == L"=") {
        return ATTR_MATCHER(t, consume<Token>());
    }
    throw SyntaxError(peek<Token>(), "Expected =");
}

NS_PREFIX SelectorParser::consumeNsPrefix() {
    Token t = consume<Token>();
    if (t.type == IDENT || (t.type == DELIM && t.lexeme == L"*")) {
        Token bar = consume(DELIM, "Expected delim");
        if (bar.lexeme == L"|") {
            return NS_PREFIX(t, bar);
        }
        throw SyntaxError(&bar, "Expected |");
    }
    else if (t.type == DELIM && t.lexeme == L"|") {
        return NS_PREFIX(nullopt, t);
    }
    else {
        throw SyntaxError(&t, "Expected |");
    }
}

WQ_NAME SelectorParser::consumeWqName() {
    Token t = consume<Token>();
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
        throw SyntaxError(peek<Token>(), "Expected NS_PREFIX or identifier");
    }
}

AttributeSelector SelectorParser::consumeAttributeSelector() {
    Token open = consume(LEFT_BRACKET, "Expected [");
    WQ_NAME name = consumeWqName();
    if (check(RIGHT_BRACKET)) {
        return {open, name, consume<Token>()};
    }
    ATTR_MATCHER matcher = consumeAttrMatcher();
    if (!check(STRING) && !check(IDENT)) {
        throw SyntaxError(peek<Token>(), "Expected string or ident");
    }
    Token tok = consume<Token>();
    if (check(RIGHT_BRACKET)) {
        return {open, name, matcher, tok, nullopt, consume<Token>()};
    }
    if (check(DELIM) && tolower((peek<Token>() -> lexeme)[0]) == 'i') {
        Token mod = consume<Token>();
        if (check(RIGHT_BRACKET)) {
            return {open, name, matcher, tok, mod, consume<Token>()};
        }
        throw SyntaxError(peek<Token>(), "Expected closing bracket");
    }
    throw SyntaxError(peek<Token>(), "Expected modifier or closing token");
}

TYPE_SELECTOR SelectorParser::consumeTypeSelector() {
    Token t = consume<Token>();
    if (t.type == DELIM && t.lexeme == L"*") {
        if (check(L"|")) {
            RECONSUME;
            NS_PREFIX prefix = consumeNsPrefix();
            if (check(L"*")) {
                return TYPE_SELECTOR(prefix, consume(DELIM, "Expected delim"));
            }
            throw SyntaxError(peek<Token>(), "Expected *");
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
    throw SyntaxError(&t, "Expected WQ_NAME or [NS_PREFIX? '*']");
}

CLASS_SELECTOR SelectorParser::consumeClassSelector() {
    if (check(DELIM) && peek<Token>() -> lexeme == L".") {
        Token dot = consume<Token>();
        if (check(IDENT)) {
            return CLASS_SELECTOR(dot, consume<Token>());
        }
        throw SyntaxError(&dot, "Expected identifier");
    }
    throw SyntaxError(peek<Token>(), "Expected .");
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
            Token t = consume<Token>();
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
                    val.push_back(t);
                    break;
                }
                case DELIM: {
                    if (t.lexeme == L"!" && !any) {
                        RECONSUME;
                        return val;
                    }
                    val.push_back(t);
                    break;
                }
                case LEFT_PAREN: case LEFT_BRACE: case LEFT_BRACKET:
                {
                    opening.push_back(t.type);
                    val.push_back(t);
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
                    val.push_back(t);
                    break;
                }
                default: {
                    val.push_back(t);
                    break;
                }
            }
        }
        else {
            val.push_back(consume<COMPONENT_VALUE>());
        }
    }
    return val;
}

PseudoClassSelector SelectorParser::consumePseudoClassSelector() {
    if (check(COLON)) {
        Token colon = consume<Token>();
        if (check(IDENT)) {
            return {colon, consume<Token>()};
        }
        else if (check(FUNCTION)) {
            Token func = consume<Token>();
            vector<COMPONENT_VALUE> any = consumeDeclarationValue(true);
            if (check(RIGHT_PAREN)) {
                return {colon, func, any, consume<Token>()};
            }
            throw SyntaxError(peek<Token>(), "Expected closing parenthesis");
        }
    }
    throw SyntaxError(peek<Token>(), "Expected colon or function");
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
        Token t = consume<Token>();
        switch (t.type) {
            case WHITESPACE: case SEMICOLON: break;
            case T_EOF: {
                std::move(decls.begin(), decls.end(), block.begin());
                std::move(rules.begin(), rules.end(), block.end());
                return block;
                break;
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
        COMPONENT_VALUE cv_slast = dec.value[dec.value.size() - 2];
        COMPONENT_VALUE cv_last = dec.value.back();
        if (std::holds_alternative<Token>(cv_slast) && std::holds_alternative<Token>(cv_last)) {
            Token t_slast = std::get<Token>(cv_slast);
            Token t_last = std::get<Token>(cv_last);
            if (t_slast.type == DELIM && t_slast.lexeme == L"!" && t_last.type == IDENT && wstrcompi(t_last.lexeme, L"important")) {
                dec.value.pop_back();
                dec.value.pop_back();
            }
        }
    }
    return dec;
}

#pragma endregion