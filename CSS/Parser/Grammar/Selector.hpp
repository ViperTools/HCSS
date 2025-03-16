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
};

// <ns-prefix>? <ident-token>
struct WqName {
    optional<NsPrefix> prefix;
    Token ident;
};

// [ '~' | '|' | '^' | '$' | '*' ]? '='
struct AttrMatcher {
    optional<Token> tok;
    Token eq;
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
        : openBracket(std::move(openBracket)),
        name(std::move(name)),
        closeBracket(std::move(closeBracket))
    {};
    AttributeSelector(Token openBracket, WqName name, AttrMatcher matcher, Token tok, optional<Token> modifier, Token closeBracket)
        : openBracket(std::move(openBracket)),
        name(std::move(name)),
        matcher(std::move(matcher)),
        tok(std::move(tok)),
        modifier(std::move(modifier)),
        closeBracket(std::move(closeBracket))
    {};
};

// <wq-name> | <ns-prefix>? '*'
struct TypeSelector {
    optional<WqName> wqName;
    optional<NsPrefix> nsPrefix;
    optional<Token> star;
    TypeSelector(WqName wqName)
        : wqName(std::move(wqName))
    {};
    TypeSelector(optional<NsPrefix> nsPrefix, Token star)
        : nsPrefix(std::move(nsPrefix)),
        star(std::move(star))
    {};
};

// <hash-token>
using IdSelector = Token;

// '.' <ident-token>
struct ClassSelector {
    Token dot;
    Token ident;
};

/*
':' <ident-token> | ':' <function-token> <any-value> ')'
*/
struct PseudoClassSelector {
    Token colon;
    Token tok;
    optional<vector<ComponentValue>> anyValue;
    optional<Token> closeParen;
};

// <id-selector> | <class-selector> | <attribute-selector> | <pseudo-class-selector>
using SubclassSelector = variant<std::monostate, IdSelector, ClassSelector, AttributeSelector, PseudoClassSelector>;

// <type-selector> | <subclass-selector>
using SimpleSelector = variant<TypeSelector, SubclassSelector>;

// ':' <pseudo-class-selector>
struct PseudoElementSelector {
    Token colon;
    PseudoClassSelector selector;
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
};

// <compound-selector> [ <combinator>? <compound-selector> ]*
using ComplexSelector = vector<pair<Combinator, CompoundSelector>>;

// <combinator>? <complex-selector>
struct RelativeSelector {
    Combinator combinator;
    ComplexSelector selector;
};

using ComplexSelectorList = vector<ComplexSelector>;