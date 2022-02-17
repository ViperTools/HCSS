#include "SelectorParser.hpp"
#include <queue>

ComplexSelectorList SelectorParser::parse() {
    skipws();
    ComplexSelectorList list;
    while (!values.empty()) {
        list.emplace_back(consumeComplexSelector());
        if (check(COMMA)) {
            values.pop_front();
            skipws();
        }
    }
    return list;
}

ComplexSelector SelectorParser::consumeComplexSelector() {
    skipws();
    ComplexSelector selectors = {{{}, consumeCompoundSelector()}};
    skipws();
    while (!values.empty() && !check(T_EOF) && !check(COMMA)) {
        Combinator comb = consumeCombinator();
        skipws();
        selectors.emplace_back(comb, consumeCompoundSelector());
        skipws();
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
    vector<SubclassSelector> subclasses;
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
                    if (check(COLON, 1)) {
                        consuming = false;
                        break;
                    }
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
            if (block && block -> open.type == LEFT_BRACKET) {
                subclasses.emplace_back(consumeSubclassSelector());
            }
        }
    }
    vector<PseudoSelectorPair> pseudos;
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
    if (!type && subclasses.empty() && pseudos.empty()) {
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

SubclassSelector SelectorParser::consumeSubclassSelector() {
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

vector<ComponentValue> SelectorParser::consumeDeclarationValue(bool any) {
    vector<ComponentValue> val;
    std::queue<TokenType> opening;
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
                    opening.emplace(t -> type);
                    val.emplace_back(*t);
                    values.pop_front();
                    break;
                }
                case RIGHT_PAREN: case RIGHT_BRACE: case RIGHT_BRACKET:
                {
                    TokenType m = mirror(opening.front());
                    opening.pop();
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
    else if (auto f = peek<FunctionCall>()) {
        auto temp = std::move(*f);
        values.pop_front();
        vector<ComponentValue> any;
        for (vector<ComponentValue> arg : temp.arguments) {
            std::move(arg.begin(), arg.end(), std::back_inserter(any));
            any.emplace_back(Token(COMMA, L","));
        }
        return {colon, temp.name, any, Token(RIGHT_PAREN, L")")};
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

SimpleSelector SelectorParser::consumeSimpleSelector() {
    SubclassSelector subclass = consumeSubclassSelector();
    if (subclass.index() > 0) {
        return subclass;
    }
    else {
        return consumeTypeSelector();
    }
}