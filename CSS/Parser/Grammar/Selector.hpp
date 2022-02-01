#pragma once

#include "../Macros.hpp"
#include <utility>
#include <vector>
#include <optional>
#include <variant>
#include <utility>
using std::vector;
using std::optional;
using std::variant;
using std::pair;
using std::nullopt;
using std::monostate;

// [ <ident-token> | '*' ]? '|'
struct NsPrefix {
    optional<Token> value;
    Token bar;
    NsPrefix(optional<Token> value, Token bar)
        : value(value),
        bar(bar)
    {};
};

// <ns-prefix>? <ident-token>
struct WqName {
    optional<NsPrefix> prefix;
    Token ident;
    WqName(optional<NsPrefix> prefix, Token ident)
        : prefix(prefix),
        ident(ident)
    {};
};

// [ '~' | '|' | '^' | '$' | '*' ]? '='
struct AttrMatcher {
    optional<Token> tok;
    Token eq;
    AttrMatcher(optional<Token> tok, Token eq)
        : tok(tok),
        eq(eq)
    {};
};

// i
#define ATTR_MODIFIER Token

// '>' | '+' | '~' | [ '|' '|' ]
#define COMBINATOR variant<monostate, Token, pair<Token, Token>>

/*
'[' <wq-name> ']' |
    [' <wq-name> <attr-matcher> [ <string-token> | <ident-token> ] <attr-modifier>? ']'
*/
class AttributeSelector {
    public:
        Token openBracket;
        WqName name;
        optional<AttrMatcher> matcher = nullopt;
        optional<Token> tok = nullopt;
        optional<Token> modifier = nullopt;
        Token closeBracket;
        AttributeSelector(Token openBracket, WqName name, Token closeBracket)
            : openBracket(std::move(openBracket)),
            name(std::move(name)),
            closeBracket(std::move(closeBracket))
        {};
        AttributeSelector(Token openBracket, WqName name, AttrMatcher matcher, Token tok, optional<Token> modifier, Token closeBracket)
            : openBracket(std::move(openBracket)),
            name(std::move(name)),
            matcher(matcher),
            tok(tok),
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
        : wqName(wqName)
    {};
    TypeSelector(optional<NsPrefix> nsPrefix, Token star)
        : nsPrefix(nsPrefix),
        star(star)
    {};
};

// <hash-token>
#define ID_SELECTOR Token

// '.' <ident-token>
struct ClassSelector {
    Token dot;
    Token ident;
    ClassSelector(Token dot, Token ident)
        : dot(dot),
        ident(ident)
    {};
};

// <id-selector> | <class-selector> | <attribute-selector> | <pseudo-class-selector>
#define SUBCLASS_SELECTOR variant<monostate, ID_SELECTOR, ClassSelector, AttributeSelector, PseudoClassSelector>

// <type-selector> | <subclass-selector>
#define SIMPLE_SELECTOR variant<TypeSelector, SUBCLASS_SELECTOR>

/*
':' <ident-token> |
    ':' <function-token> <any-value> ')'
*/

struct PseudoClassSelector {
    public:
        Token colon;
        Token tok;
        optional<vector<COMPONENT_VALUE>> anyValue;
        optional<Token> closeParen;
        PseudoClassSelector(Token colon, Token tok, optional<vector<COMPONENT_VALUE>> anyValue = nullopt, optional<Token> closeParen = nullopt)
            : colon(std::move(colon)),
            tok(std::move(tok)),
            anyValue(std::move(anyValue)),
            closeParen(std::move(closeParen))
        {};
};

// ':' <pseudo-class-selector>
struct PseudoElementSelector {
    Token colon;
    PseudoClassSelector selector;
    PseudoElementSelector(Token colon, PseudoClassSelector selector)
        : colon(colon),
        selector(selector)
    {};
};

/*
[ <type-selector>? <subclass-selector>*
    [ <pseudo-element-selector> <pseudo-class-selector>* ]* ]!
*/
#define PSEUDO_SELECTOR_PAIR pair<PseudoElementSelector, vector<PseudoClassSelector>>
class CompoundSelector {
    public:
        optional<TypeSelector> typeSelector;
        vector<SUBCLASS_SELECTOR> subclassSelectors;
        vector<PSEUDO_SELECTOR_PAIR> pseudoSelectors;
        explicit CompoundSelector(optional<TypeSelector> typeSelector = nullopt, vector<SUBCLASS_SELECTOR> subclassSelectors = {}, const vector<PSEUDO_SELECTOR_PAIR>& pseudoSelectors = {})
            : typeSelector(std::move(typeSelector)),
            subclassSelectors(std::move(subclassSelectors)),
            pseudoSelectors(pseudoSelectors)
        {};
};

// <compound-selector> [ <combinator>? <compound-selector> ]*
#define COMPLEX_SELECTOR_PAIR pair<COMBINATOR, CompoundSelector>
#define COMPLEX_SELECTOR vector<COMPLEX_SELECTOR_PAIR>

// <combinator>? <complex-selector>
class RelativeSelector {
    public:
        COMBINATOR combinator;
        COMPLEX_SELECTOR selector;
        RelativeSelector(COMBINATOR combinator, COMPLEX_SELECTOR selector)
            : selector(std::move(selector)),
            combinator(std::move(combinator))
        {};
};

#define SELECTOR_LIST vector<COMPLEX_SELECTOR>
#define COMPLEX_SELECTOR_LIST vector<COMPLEX_SELECTOR>
#define COMPOUND_SELECTOR_LIST vector<CompoundSelector>
#define SIMPLE_SELECTOR_LIST vector<SIMPLE_SELECTOR>
#define RELATIVE_SELECTOR_LIST vector<RelativeSelector>