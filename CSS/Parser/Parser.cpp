// TODO Handle whitespace based off of CSS spec

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

#pragma region Style Sheet Parser
#define NOT_EOF idx < tokens.size() && tokens[idx].type != T_EOF

Token* StyleSheetParser::peek() {
    return idx >= 0 && idx < tokens.size() ? &tokens[idx] : nullptr;
}

bool StyleSheetParser::check(TokenType type) {
    Token* t = peek();
    if (t == nullptr) {
        return false;
    }
    return t->type == type;
}

Token StyleSheetParser::consume(TokenType type, const string& error) {
    if (!check(type)) {
        throw SyntaxError(peek(), error);
    }
    return consume();
}

Token StyleSheetParser::consume() {
    Token* t = peek();
    if (t == nullptr || t -> type == T_EOF) {
        throw SyntaxError(nullptr, "No tokens left to consume.");
    }
    idx++;
    return *t;
}

vector<SYNTAX_NODE> StyleSheetParser::parse() {
    rules = consumeRulesList();
    top = false;
    for (SYNTAX_NODE node : rules) {
        if (std::holds_alternative<QualifiedRule>(node)) {
            QualifiedRule rule = std::get<QualifiedRule>(node);
            SelectorParser(rule).parse();
        }
    }
    return rules;
}

vector<SYNTAX_NODE> StyleSheetParser::consumeRulesList() {
    vector<SYNTAX_NODE> list;
    while (NOT_EOF) {
        Token t = consume();
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

COMPONENT_VALUE StyleSheetParser::consumeComponentValue() {
    Token t = consume();
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

AtRule StyleSheetParser::consumeAtRule() {
    Token at = consume(AT_KEYWORD, "Expected AT_KEYWORD");
    AtRule rule(at);
    while (NOT_EOF) {
        Token t = consume();
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

Function StyleSheetParser::consumeFunction() {
    Token ident = consume(IDENT, "Expected identifier");
    Function f(ident);
    while (NOT_EOF) {
        Token t = consume();
        switch (t.type) {
            case T_EOF:
            case RIGHT_PAREN: return f;
            default: {
                f.components.push_back(consumeComponentValue());
            }
        }
    }
    return f;
}

QualifiedRule StyleSheetParser::consumeQualifiedRule() {
    QualifiedRule rule;
    while (NOT_EOF) {
        Token t = consume();
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

SimpleBlock StyleSheetParser::consumeSimpleBlock() {
    Token open = LAST;
    SimpleBlock block(open);
    TokenType m = mirror(open.type);
    vector<COMPONENT_VALUE> components;
    while (NOT_EOF) {
        Token t = consume();
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
#undef NOT_EOF
#define NOT_EOF idx < rule.components.size() && !checkToken(T_EOF)

COMPONENT_VALUE* SelectorParser::peek() {
    return idx >= 0 && idx < rule.components.size() ? &rule.components[idx] : nullptr;
}

COMPONENT_VALUE SelectorParser::consume() {
    COMPONENT_VALUE* val = peek();
    if (val != nullptr) {
        idx++;
        return *val;
    }
    else {
        throw SyntaxError(nullptr, "No components left to consume.");
    }
}

Token* SelectorParser::peekToken() {
    COMPONENT_VALUE* val = peek();
    if (val != nullptr && std::holds_alternative<Token>(*val)) {
        return &std::get<Token>(*val);
    }
    return nullptr;
}

Token SelectorParser::consumeToken() {
    COMPONENT_VALUE val = consume();
    if (std::holds_alternative<Token>(val)) {
        return std::get<Token>(val);
    }
    throw SyntaxError(nullptr, "Expected token");
}

Token SelectorParser::consumeToken(TokenType type, const string& error) {
    Token t = consumeToken();
    if (t.type == type) {
        return t;
    }
    throw SyntaxError(&t, error);
}

bool SelectorParser::isToken() {
    COMPONENT_VALUE* val = peek();
    if (val == nullptr) return false;
    return std::holds_alternative<Token>(*val);
}

bool SelectorParser::checkToken(TokenType type) {
    COMPONENT_VALUE* val = peek();
    if (val != nullptr && std::holds_alternative<Token>(*val)) {
        Token t = std::get<Token>(*val);
        return t.type == type;
    }
    return false;
}

bool SelectorParser::checkToken(const wstring& lexeme) {
    COMPONENT_VALUE* val = peek();
    if (val != nullptr && std::holds_alternative<Token>(*val)) {
        Token t = std::get<Token>(*val);
        return t.lexeme == lexeme;
    }
    return false;
}

COMPLEX_SELECTOR_LIST SelectorParser::parse() {
    IGNORE_WHITESPACE;
    COMPLEX_SELECTOR_LIST list;
    while (NOT_EOF) {
        list.push_back(consumeComplexSelector());
    }
    return list;
}

ComplexSelector SelectorParser::consumeComplexSelector() {
    vector<COMPLEX_SELECTOR_PAIR> selectors = {COMPLEX_SELECTOR_PAIR({ }, consumeCompoundSelector()) };
    while (NOT_EOF) {
        IGNORE_WHITESPACE;
        COMBINATOR comb = consumeCombinator();
        IGNORE_WHITESPACE;
        selectors.emplace_back(comb, consumeCompoundSelector());
    }
    return ComplexSelector(vector<COMPLEX_SELECTOR_PAIR>());
}

COMBINATOR SelectorParser::consumeCombinator() {
    Token tok = consumeToken();
    switch (tok.lexeme[0]) {
        case '>':
        case '+':
        case '~': return tok;
        case '|': {
            Token t2 = consumeToken(DELIM, "Expected DELIM");
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
    if (checkToken(IDENT) || peekToken() -> lexeme == L"*") {
        type = consumeTypeSelector();
    }
    vector<SUBCLASS_SELECTOR> subclasses;
    bool consuming = true;
    while (consuming && NOT_EOF) {
        Token* t = peekToken();
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
                bool isColon = checkToken(COLON);
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
    while (checkToken(COLON)) {
        Token colon = consumeToken();
        if (checkToken(COLON)) {
            PSEUDO_ELEMENT_SELECTOR el = consumePseudoElementSelector();
            vector<PseudoClassSelector> classes;
            while (checkToken(COLON)) {
                Token t = consumeToken();
                if (!checkToken(COLON)) {
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
        throw SyntaxError(peekToken(), "At least one value is required");
    }
    return CompoundSelector(type, subclasses, pseudos);
}

ATTR_MATCHER SelectorParser::consumeAttrMatcher() {
    Token t = consumeToken(DELIM, "Expected delim");
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
    if (checkToken(DELIM) && peekToken() -> lexeme == L"=") {
        return ATTR_MATCHER(t, consumeToken());
    }
    throw SyntaxError(peekToken(), "Expected =");
}

NS_PREFIX SelectorParser::consumeNsPrefix() {
    Token t = consumeToken();
    if (t.type == IDENT || (t.type == DELIM && t.lexeme == L"*")) {
        Token bar = consumeToken(DELIM, "Expected delim");
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
    Token t = consumeToken();
    if (t.type == IDENT) {
        if (checkToken(DELIM) && peekToken() -> lexeme == L"|") {
            RECONSUME;
            return WQ_NAME(consumeNsPrefix(), consumeToken(IDENT, "Expected identifier"));
        }
        return WQ_NAME(nullopt, t);
    }
    else if (t.lexeme == L"*") {
        RECONSUME;
        return WQ_NAME(consumeNsPrefix(), consumeToken(IDENT, "Expected identifier"));
    }
    else {
        throw SyntaxError(peekToken(), "Expected NS_PREFIX or identifier");
    }
}

AttributeSelector SelectorParser::consumeAttributeSelector() {
    Token open = consumeToken(LEFT_BRACKET, "Expected [");
    WQ_NAME name = consumeWqName();
    if (checkToken(RIGHT_BRACKET)) {
        return {open, name, consumeToken()};
    }
    ATTR_MATCHER matcher = consumeAttrMatcher();
    if (!checkToken(STRING) && !checkToken(IDENT)) {
        throw SyntaxError(peekToken(), "Expected string or ident");
    }
    Token tok = consumeToken();
    if (checkToken(RIGHT_BRACKET)) {
        return {open, name, matcher, tok, nullopt, consumeToken()};
    }
    if (checkToken(DELIM) && tolower((peekToken() -> lexeme)[0]) == 'i') {
        Token mod = consumeToken();
        if (checkToken(RIGHT_BRACKET)) {
            return {open, name, matcher, tok, mod, consumeToken()};
        }
        throw SyntaxError(peekToken(), "Expected closing bracket");
    }
    throw SyntaxError(peekToken(), "Expected modifier or closing token");
}

TYPE_SELECTOR SelectorParser::consumeTypeSelector() {
    Token t = consumeToken();
    if (t.type == DELIM && t.lexeme == L"*") {
        if (checkToken(L"|")) {
            RECONSUME;
            NS_PREFIX prefix = consumeNsPrefix();
            if (checkToken(L"*")) {
                return TYPE_SELECTOR(prefix, consumeToken(DELIM, "Expected delim"));
            }
            throw SyntaxError(peekToken(), "Expected *");
        }
        else {
            return TYPE_SELECTOR(nullopt, t);
        }
    }
    else if (t.type == IDENT) {
        if (checkToken(L"|")) {
            RECONSUME;
            NS_PREFIX prefix = consumeNsPrefix();
            if (checkToken(L"*")) {
                return TYPE_SELECTOR(prefix, consumeToken(DELIM, "Expected delim"));
            }
            else if (checkToken(IDENT)) {
                return WQ_NAME(prefix, consumeToken());
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
    if (checkToken(DELIM) && peekToken() -> lexeme == L".") {
        Token dot = consumeToken();
        if (checkToken(IDENT)) {
            return CLASS_SELECTOR(dot, consumeToken());
        }
        throw SyntaxError(&dot, "Expected identifier");
    }
    throw SyntaxError(peekToken(), "Expected .");
}

SUBCLASS_SELECTOR SelectorParser::consumeSubclassSelector() {
    if (checkToken(HASH)) {
        return consumeToken();
    }
    else if (checkToken(DELIM) && peekToken() -> lexeme == L".") {
        return consumeClassSelector();
    }
    else if (checkToken(LEFT_BRACKET)) {
        return consumeAttributeSelector();
    }
    else if (checkToken(COLON)) {
        return consumePseudoClassSelector();
    }
    return { };
}

vector<Token> SelectorParser::consumeDeclarationValue(bool any) {
    vector<Token> val;
    while (isToken()) {
        Token t = consumeToken();
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
    return val;
}

PseudoClassSelector SelectorParser::consumePseudoClassSelector() {
    if (checkToken(COLON)) {
        Token colon = consumeToken();
        if (checkToken(IDENT)) {
            return {colon, consumeToken()};
        }
        else if (checkToken(FUNCTION)) {
            Token func = consumeToken();
            vector<Token> any = consumeDeclarationValue(true);
            if (checkToken(RIGHT_PAREN)) {
                return {colon, func, any, consumeToken()};
            }
            throw SyntaxError(peekToken(), "Expected closing parenthesis");
        }
    }
    throw SyntaxError(peekToken(), "Expected colon or function");
}

PSEUDO_ELEMENT_SELECTOR SelectorParser::consumePseudoElementSelector() {
    Token colon = consumeToken(COLON, "Expected :");
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