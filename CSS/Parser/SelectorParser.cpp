#include "SelectorParser.hpp"

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
        Combinator comb = consumeCombinator();
        IGNORE_WHITESPACE;
        selectors.emplace_back(comb, consumeCompoundSelector());
        IGNORE_WHITESPACE;
    }
    return selectors;
}

Combinator SelectorParser::consumeCombinator() {
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
            auto t1 = std::move(*tok);
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
    while (!values.empty()) {
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
    return val;
}

PseudoClassSelector SelectorParser::consumePseudoClassSelector() {
    auto colon = consume(COLON, "Expected colon");
    if (auto t = peek<Token>()) {
        auto temp = std::move(*t);
        values.pop_front();
        switch (t -> type) {
            case IDENT: return {colon, temp};
            case FUNCTION: return {colon, temp, consumeDeclarationValue(true), consume(RIGHT_PAREN, "Expected closing parenthesis")};
            default: break;
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
    Combinator comb = consumeCombinator();
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