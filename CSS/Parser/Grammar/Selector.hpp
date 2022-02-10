#pragma once

#include "../Types.hpp"
#include <utility>
#include <vector>
#include <optional>
#include <variant>
using std::vector;
using std::optional;
using std::variant;
using std::pair;
using std::nullopt;
using std::move;

// [ <ident-token> | '*' ]? '|'
struct NsPrefix {
    optional<Token> value;
    Token bar;
    NsPrefix(optional<Token> value, Token bar)
        : value(move(value)),
        bar(move(bar))
    {};
};

// <ns-prefix>? <ident-token>
struct WqName {
    optional<NsPrefix> prefix;
    Token ident;
    WqName(optional<NsPrefix> prefix, Token ident)
        : prefix(move(prefix)),
        ident(move(ident))
    {};
};

// [ '~' | '|' | '^' | '$' | '*' ]? '='
struct AttrMatcher {
    optional<Token> tok;
    Token eq;
    AttrMatcher(optional<Token> tok, Token eq)
        : tok(move(tok)),
        eq(move(eq))
    {};
};

// '>' | '+' | '~' | [ '|' '|' ]
using Combinator = variant<std::monostate, Token, pair<Token, Token>>;

/*
'[' <wq-name> ']' |
    [' <wq-name> <attr-matcher> [ <string-token> | <ident-token> ] <attr-modifier>? ']'
*/
struct AttributeSelector {
    Token openBracket;
    WqName name;
    optional<AttrMatcher> matcher;
    optional<Token> tok;
    optional<Token> modifier;
    Token closeBracket;
    AttributeSelector(Token openBracket, WqName name, Token closeBracket)
        : openBracket(move(openBracket)),
        name(move(name)),
        closeBracket(move(closeBracket))
    {};
    AttributeSelector(Token openBracket, WqName name, AttrMatcher matcher, Token tok, optional<Token> modifier, Token closeBracket)
        : openBracket(move(openBracket)),
        name(move(name)),
        matcher(move(matcher)),
        tok(move(tok)),
        modifier(move(modifier)),
        closeBracket(move(closeBracket))
    {};
};

// <wq-name> | <ns-prefix>? '*'
struct TypeSelector {
    optional<WqName> wqName;
    optional<NsPrefix> nsPrefix;
    optional<Token> star;
    TypeSelector(WqName wqName)
        : wqName(move(wqName))
    {};
    TypeSelector(optional<NsPrefix> nsPrefix, Token star)
        : nsPrefix(move(nsPrefix)),
        star(move(star))
    {};
};

// <hash-token>
using IdSelector = Token;

// '.' <ident-token>
struct ClassSelector {
    Token dot;
    Token ident;
    ClassSelector(Token dot, Token ident)
        : dot(move(dot)),
        ident(move(ident))
    {};
};

/*
':' <ident-token> | ':' <function-token> <any-value> ')'
*/
struct PseudoClassSelector {
    Token colon;
    Token tok;
    optional<vector<ComponentValue>> anyValue;
    optional<Token> closeParen;
    PseudoClassSelector(Token colon, Token tok, optional<vector<ComponentValue>> anyValue = {}, optional<Token> closeParen = {})
        : colon(move(colon)),
        tok(move(tok)),
        anyValue(std::move(anyValue)),
        closeParen(move(closeParen))
    {};
};

// <id-selector> | <class-selector> | <attribute-selector> | <pseudo-class-selector>
using SubclassSelector = variant<std::monostate, IdSelector, ClassSelector, AttributeSelector, PseudoClassSelector>;

// <type-selector> | <subclass-selector>
using SimpleSelector = variant<TypeSelector, SubclassSelector>;

// ':' <pseudo-class-selector>
struct PseudoElementSelector {
    Token colon;
    PseudoClassSelector selector;
    PseudoElementSelector(Token colon, PseudoClassSelector selector)
        : colon(move(colon)),
        selector(move(selector))
    {};
};

/*
[ <type-selector>? <subclass-selector>*
    [ <pseudo-element-selector> <pseudo-class-selector>* ]* ]!
*/
using PseudoSelectorPair = pair<PseudoElementSelector, vector<PseudoClassSelector>>;
struct CompoundSelector {
    optional<TypeSelector> typeSelector;
    vector<SubclassSelector> subclassSelectors;
    vector<PseudoSelectorPair> pseudoSelectors;
    explicit CompoundSelector(optional<TypeSelector> typeSelector = nullopt, vector<SubclassSelector> subclassSelectors = {}, const vector<PseudoSelectorPair>& pseudoSelectors = {})
        : typeSelector(move(typeSelector)),
        subclassSelectors(move(subclassSelectors)),
        pseudoSelectors(move(pseudoSelectors))
    {};
};

// <compound-selector> [ <combinator>? <compound-selector> ]*
using ComplexSelector = vector<pair<Combinator, CompoundSelector>>;

// <combinator>? <complex-selector>
struct RelativeSelector {
    Combinator combinator;
    ComplexSelector selector;
    RelativeSelector(Combinator combinator, ComplexSelector selector)
        : selector(move(selector)),
        combinator(move(combinator))
    {};
};

using ComplexSelectorList = vector<ComplexSelector>;