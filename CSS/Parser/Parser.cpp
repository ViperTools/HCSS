// TODO Handle whitespace based off of CSS spec

#include "Parser.hpp"
#include "Errors/SyntaxError.hpp"
#include "../Lexer/TokenType.hpp"
#include <iostream>
#include <deque>
#include <cctype>

// Helpers
TokenType Mirror(TokenType type) {
    switch (type) {
        case LEFT_BRACE: return RIGHT_BRACE;
        case LEFT_BRACKET: return RIGHT_BRACKET;
        case LEFT_PAREN: return RIGHT_PAREN;
        default: throw SyntaxError(NULL, "Expected {, [, or (");
    }
}

// STYLE SHEET PARSER (Entrypoint)

Token* StyleSheetParser::Peek() {
    return idx >= 0 && idx < tokens.size() ? &tokens[idx] : NULL;
}

bool StyleSheetParser::Check(TokenType type) {
    Token* t = Peek();
    if (t == NULL) {
        return false;
    }
    return t->type == type;
}

Token StyleSheetParser::Consume(TokenType type, string error) {
    if (!Check(type)) {
        throw SyntaxError(Peek(), error);
    }
    return Consume();
}

Token StyleSheetParser::Consume() {
    Token* t = Peek();
    if (t == NULL) {
        throw SyntaxError(NULL, "No tokens left to consume.");
    }
    idx++;
    return *t;
}

vector<SyntaxNode> StyleSheetParser::Parse() {
    rules = ConsumeRulesList();
    for (SyntaxNode node : rules) {
        if (std::holds_alternative<QualifiedRule>(node)) {
            QualifiedRule rule = std::get<QualifiedRule>(node);
            SelectorParser(rule).Parse();
        }
    }
    return rules;
}

vector<SyntaxNode> StyleSheetParser::ConsumeRulesList() {
    vector<SyntaxNode> rules;
    while (Peek() != NULL) {
        Token t = Consume();
        switch (t.type) {
            case WHITESPACE: break;
            case T_EOF: {
                return rules;
            }
            case CDO:
            case CDC: {
                if (!top) {
                    rules.push_back(ConsumeQualifiedRule());
                }
                break;
            }
            case AT_KEYWORD: {
                Reconsume;
                rules.push_back(ConsumeAtRule());
                break;
            }
            default: {
                Reconsume;
                rules.push_back(ConsumeQualifiedRule());
            }
        }
    }
    return rules;
}

ComponentValue StyleSheetParser::ConsumeComponentValue() {
    Token t = Consume();
    switch (t.type) {
        case LEFT_BRACE: case LEFT_BRACKET: case LEFT_PAREN: {
            return ConsumeSimpleBlock();
        }
        case FUNCTION: {
            return ConsumeFunction();
        }
        default: {
            return t;
        }
    }
}

AtRule StyleSheetParser::ConsumeAtRule() {
    Token at = Consume(AT_KEYWORD, "Expected AT_KEYWORD");
    AtRule rule(at);
    while (Peek() != NULL) {
        Token t = Consume();
        switch (t.type) {
            case T_EOF:
            case SEMICOLON: return rule;
            case LEFT_BRACE: {
                rule.block = ConsumeSimpleBlock();
                return rule;
            }
            default: {
                Reconsume;
                rule.components.push_back(ConsumeComponentValue());
            }
        }
    }
    return rule;
}

Function StyleSheetParser::ConsumeFunction() {
    Token ident = Consume(IDENT, "Expected identifier");
    Function f(ident);
    while (Peek() != NULL) {
        Token t = Consume();
        switch (t.type) {
            case T_EOF:
            case RIGHT_PAREN: return f;
            default: {
                f.components.push_back(ConsumeComponentValue());
            }
        }
    }
    return f;
}

QualifiedRule StyleSheetParser::ConsumeQualifiedRule() {
    QualifiedRule rule;
    while (Peek() != NULL) {
        Token t = Consume();
        switch (t.type) {
            case T_EOF: return rule; //NULL;
            case LEFT_BRACE: {
                rule.block = ConsumeSimpleBlock();
                return rule;
            }
            default: {
                Reconsume;
                rule.components.push_back(ConsumeComponentValue());
            }
        }
    }
    return rule;
}

SimpleBlock StyleSheetParser::ConsumeSimpleBlock() {
    Token open = Last;
    SimpleBlock block(open);
    TokenType mirror = Mirror(open.type);
    vector<ComponentValue> components;
    while (Peek() != NULL) {
        Token t = Consume();
        if (t.type == T_EOF || t.type == mirror) {
            return block;
        }
        else {
            Reconsume;
            block.components.push_back(ConsumeComponentValue());
        }
    }
    return block;
}

// SELECTOR PARSER

ComponentValue* SelectorParser::Peek() {
    std::cout << "peek" << std::endl;
    return idx >= 0 && idx < rule.components.size() ? &rule.components[idx] : NULL;
}

ComponentValue SelectorParser::Consume() {
    ComponentValue* val = Peek();
    if (val != NULL) {
        idx++;
        return *val;
    }
    else {
        throw SyntaxError(NULL, "No components left to consume.");
    }
}

Token* SelectorParser::PeekToken() {
    ComponentValue* val = Peek();
    if (val != NULL && std::holds_alternative<Token>(*val)) {
        return &std::get<Token>(*val);
    }
    return NULL;
}

Token SelectorParser::ConsumeToken() {
    ComponentValue val = Consume();
    if (std::holds_alternative<Token>(val)) {
        return std::get<Token>(val);
    }
    throw SyntaxError(NULL, "Expected token");
}

Token SelectorParser::ConsumeToken(TokenType type, string error) {
    Token t = ConsumeToken();
    if (t.type == type) {
        return t;
    }
    throw SyntaxError(&t, error);
}

bool SelectorParser::IsToken() {
    ComponentValue* val = Peek();
    if (val == NULL) return false;
    return std::holds_alternative<Token>(*val);
}

bool SelectorParser::CheckToken(TokenType type) {
    ComponentValue* val = Peek();
    if (val != NULL && std::holds_alternative<Token>(*val)) {
        Token t = std::get<Token>(*val);
        return t.type == type;
    }
    return false;
}

bool SelectorParser::CheckToken(string lexeme) {
    ComponentValue* val = Peek();
    if (val != NULL && std::holds_alternative<Token>(*val)) {
        Token t = std::get<Token>(*val);
        return t.lexeme == lexeme;
    }
    return false;
}

ComplexSelectorList SelectorParser::Parse() {
    ComplexSelectorList list;
    while (Peek() != NULL) {
        std::cout << "complex" << std::endl;
        list.push_back(ConsumeComplexSelector());
    }
    return list;
}

ComplexSelector SelectorParser::ConsumeComplexSelector() {
    vector<ComplexSelectorPair> selectors = { ComplexSelectorPair({ }, ConsumeCompoundSelector()) };
    std::cout << "start" << std::endl;
    while (PeekToken() != NULL) {
        std::cout << "tok" << std::endl;
        selectors.push_back(ComplexSelectorPair(ConsumeCombinator(), ConsumeCompoundSelector()));
    }
    std::cout << "done" << std::endl;
    return ComplexSelector(selectors);
}

Combinator SelectorParser::ConsumeCombinator() {
    Token tok = ConsumeToken();
    switch (tok.lexeme[0]) {
        case '>':
        case '+':
        case '~': return tok;
        case '|': {
            Token t2 = ConsumeToken(DELIM, "Expected DELIM");
            if (t2.lexeme != "|") {
                throw SyntaxError(&t2, "Expected second |");
            }
            return pair<Token, Token>(tok, t2);
        }
        default: {
            Reconsume;
            return { };
        }
    }
}

CompoundSelector SelectorParser::ConsumeCompoundSelector() {
    optional<TypeSelector> type = nullopt;
    if (CheckToken(IDENT) || PeekToken() -> lexeme == "*") {
        type = ConsumeTypeSelector();
    }
    vector<SubclassSelector> subclasses;
    bool consuming = true;
    while (consuming && PeekToken() != NULL) {
        std::cout << "consuming" << std::endl;
        Token* t = PeekToken();
        switch (t -> type) {
            case DELIM: {
                if (t -> lexeme != ".") {
                    consuming = false;
                    break;
                }
            }
            case COLON: {
                ConsumeToken();
                bool isColon = CheckToken(COLON);
                Reconsume;
                if (isColon) {
                    consuming = false;
                    break;
                }
                std::cout << "colon" << std::endl;
            }
            case HASH:
            case LEFT_BRACKET: {
                subclasses.push_back(ConsumeSubclassSelector());
                std::cout << "added" << std::endl;
                break;
            }
            default: {
                consuming = false;
            }
        }
    }
    std::cout << "out" << std::endl;
    vector<PseudoSelectorPair> pseudos;
    while (CheckToken(COLON)) {
        std::cout << "c" << std::endl;
        Token colon = ConsumeToken();
        if (CheckToken(COLON)) {
            PseudoElementSelector el = ConsumePseudoElementSelector();
            vector<PseudoClassSelector> classes;
            while (CheckToken(COLON)) {
                Token t = ConsumeToken();
                if (!CheckToken(COLON)) {
                    Reconsume;
                    classes.push_back(ConsumePseudoClassSelector());
                }
                else {
                    Reconsume;
                    break;
                }
            }
            pseudos.push_back(PseudoSelectorPair(el, classes));
        }
        else {
            Reconsume;
            break;
        }
    }
    if (!type.has_value() && subclasses.size() == 0 && pseudos.size() == 0) {
        throw SyntaxError(PeekToken(), "At least one value is required");
    }
    std::cout << "ret" << std::endl;
    return CompoundSelector(type, subclasses, pseudos);
}

AttrMatcher SelectorParser::ConsumeAttrMatcher() {
    Token t = ConsumeToken(DELIM, "Expected delim");
    switch (t.lexeme[0]) {
        case '~':
        case '|':
        case '^':
        case '$':
        case '*': break;
        case '=': return AttrMatcher(nullopt, t);
        default: {
            throw SyntaxError(&t, "Expected ~, |, ^, $, *, or =");
        }
    }
    if (CheckToken(DELIM) && PeekToken() -> lexeme == "=") {
        return AttrMatcher(t, ConsumeToken());
    }
    throw SyntaxError(PeekToken(), "Expected =");
}

NsPrefix SelectorParser::ConsumeNsPrefix() {
    Token t = ConsumeToken();
    if (t.type == IDENT || (t.type == DELIM && t.lexeme == "*")) {
        Token bar = ConsumeToken(DELIM, "Expected delim");
        if (bar.lexeme == "|") {
            return NsPrefix(t, bar);
        }
        throw SyntaxError(&bar, "Expected |");
    }
    else if (t.type == DELIM && t.lexeme == "|") {
        return NsPrefix(nullopt, t);
    }
    throw SyntaxError(&t, "Expected |");
}

WqName SelectorParser::ConsumeWqName() {
    Token t = ConsumeToken();
    if (t.type == IDENT) {
        if (CheckToken(DELIM) && PeekToken() -> lexeme == "|") {
            Reconsume;
            return WqName(ConsumeNsPrefix(), ConsumeToken(IDENT, "Expected identifier"));
        }
        return WqName(nullopt, t);
    }
    else if (t.lexeme == "*") {
        Reconsume;
        return WqName(ConsumeNsPrefix(), ConsumeToken(IDENT, "Expected identifier"));
    }
    throw SyntaxError(PeekToken(), "Expected NsPrefix or identifier");
}

AttributeSelector SelectorParser::ConsumeAttributeSelector() {
    Token open = ConsumeToken(LEFT_BRACKET, "Expected [");
    WqName name = ConsumeWqName();
    if (CheckToken(RIGHT_BRACKET)) {
        return AttributeSelector(open, name, ConsumeToken());
    }
    AttrMatcher matcher = ConsumeAttrMatcher();
    if (!CheckToken(STRING) && !CheckToken(IDENT)) {
        throw SyntaxError(PeekToken(), "Expected string or ident");
    }
    Token tok = ConsumeToken();
    if (CheckToken(RIGHT_BRACKET)) {
        return AttributeSelector(open, name, matcher, tok, nullopt, ConsumeToken());
    }
    if (CheckToken(DELIM) && tolower((PeekToken() -> lexeme)[0]) == 'i') {
        Token mod = ConsumeToken();
        if (CheckToken(RIGHT_BRACKET)) {
            return AttributeSelector(open, name, matcher, tok, mod, ConsumeToken());
        }
        throw SyntaxError(PeekToken(), "Expected closing bracket");
    }
    throw SyntaxError(PeekToken(), "Expected modifier or closing token");
}

TypeSelector SelectorParser::ConsumeTypeSelector() {
    Token t = ConsumeToken();
    if (t.type == DELIM && t.lexeme == "*") {
        if (CheckToken("|")) {
            Reconsume;
            NsPrefix prefix = ConsumeNsPrefix();
            if (CheckToken("*")) {
                return TypeSelector(prefix, ConsumeToken(DELIM, "Expected delim"));
            }
            throw SyntaxError(PeekToken(), "Expected *");
        }
        else {
            return TypeSelector(nullopt, t);
        }
    }
    else if (t.type == IDENT) {
        if (CheckToken("|")) {
            Reconsume;
            NsPrefix prefix = ConsumeNsPrefix();
            if (CheckToken("*")) {
                return TypeSelector(prefix, ConsumeToken(DELIM, "Expected delim"));
            }
            else if (CheckToken(IDENT)) {
                return WqName(prefix, ConsumeToken());
            }
        }
        else {
            return ConsumeWqName();
        }
    }
    throw SyntaxError(&t, "Expected WqName or [NsPrefix? '*']");
}

IdSelector SelectorParser::ConsumeIdSelector() {
    return ConsumeToken(HASH, "Expected ID selector (HASH)");
}

ClassSelector SelectorParser::ConsumeClassSelector() {
    if (CheckToken(DELIM) && PeekToken() -> lexeme == ".") {
        Token dot = ConsumeToken();
        if (CheckToken(IDENT)) {
            return ClassSelector(dot, ConsumeToken());
        }
        throw SyntaxError(&dot, "Expected identifier");
    }
    throw SyntaxError(PeekToken(), "Expected .");
}

SubclassSelector SelectorParser::ConsumeSubclassSelector() {
    if (CheckToken(HASH)) {
        return ConsumeToken();
    }
    else if (CheckToken(DELIM) && PeekToken() -> lexeme == ".") {
        return ConsumeClassSelector();
    }
    else if (CheckToken(LEFT_BRACKET)) {
        return ConsumeAttributeSelector();
    }
    else if (CheckToken(COLON)) {
        return ConsumePseudoClassSelector();
    }
    return { };
}

vector<Token> SelectorParser::ConsumeDeclarationValue(bool any) {
    vector<Token> val;
    while (IsToken()) {
        Token t = ConsumeToken();
        std::deque<TokenType> opening;
        switch (t.type) {
            case BAD_STRING: case BAD_URL:
            {
                return val;
            }
            case SEMICOLON: {
                if (!any) {
                    Reconsume;
                    return val;
                }
                val.push_back(t);
            }
            case DELIM: {
                if (t.lexeme == "!" && !any) {
                    Reconsume;
                    return val;
                }
                val.push_back(t);
            }
            case LEFT_PAREN: case LEFT_BRACE: case LEFT_BRACKET:
            {
                opening.push_back(t.type);
                val.push_back(t);
                break;
            }
            case RIGHT_PAREN: case RIGHT_BRACE: case RIGHT_BRACKET:
            {
                TokenType mirror = Mirror(opening.back());
                opening.pop_back();
                if (t.type != mirror) {
                    Reconsume;
                    return val;
                }
                val.push_back(t);
                break;
            }
            default: {
                val.push_back(t);
            }
        }
    }
    return val;
}

PseudoClassSelector SelectorParser::ConsumePseudoClassSelector() {
    if (CheckToken(COLON)) {
        Token colon = ConsumeToken();
        if (CheckToken(IDENT)) {
            return PseudoClassSelector(colon, ConsumeToken());
        }
        else if (CheckToken(FUNCTION)) {
            Token func = ConsumeToken();
            vector<Token> any = ConsumeDeclarationValue(true);
            if (CheckToken(RIGHT_PAREN)) {
                return PseudoClassSelector(colon, func, any, ConsumeToken());
            }
            throw SyntaxError(PeekToken(), "Expected closing parenthesis");
        }
    }
    throw SyntaxError(PeekToken(), "Expected colon or function");
}

PseudoElementSelector SelectorParser::ConsumePseudoElementSelector() {
    Token colon = ConsumeToken(COLON, "Expected :");
    return PseudoElementSelector(colon, ConsumePseudoClassSelector());
}

RelativeSelector SelectorParser::ConsumeRelativeSelector() {
    Combinator comb = ConsumeCombinator();
    return RelativeSelector(comb, ConsumeComplexSelector());
}

SimpleSelector SelectorParser::ConsumeSimpleSelector() {
    SubclassSelector subclass = ConsumeSubclassSelector();
    if (subclass.index() > 0) {
        return subclass;
    }
    else {
        return ConsumeTypeSelector();
    }
}