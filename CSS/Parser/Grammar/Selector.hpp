// Switch #define to typedef when possible

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
typedef variant<monostate, Token, pair<Token, Token>> Combinator;

/*
'[' <wq-name> ']' |
    [' <wq-name> <attr-matcher> [ <string-token> | <ident-token> ] <attr-modifier>? ']'
*/
struct AttributeSelector {
    Token openBracket;
    WqName name;
    optional<AttrMatcher> matcher = nullopt;
    optional<Token> tok = nullopt;
    optional<Token> modifier = nullopt;
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
typedef Token IdSelector;

// '.' <ident-token>
struct ClassSelector {
    Token dot;
    Token ident;
    ClassSelector(Token dot, Token ident)
        : dot(move(dot)),
        ident(move(ident))
    {};
};

// <id-selector> | <class-selector> | <attribute-selector> | <pseudo-class-selector>
#define SUBCLASS_SELECTOR variant<monostate, IdSelector, ClassSelector, AttributeSelector, PseudoClassSelector>

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
            : colon(move(colon)),
            tok(move(tok)),
            anyValue(std::move(anyValue)),
            closeParen(move(closeParen))
        {};
};

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
#define PSEUDO_SELECTOR_PAIR pair<PseudoElementSelector, vector<PseudoClassSelector>>
struct CompoundSelector {
    optional<TypeSelector> typeSelector;
    vector<SUBCLASS_SELECTOR> subclassSelectors;
    vector<PSEUDO_SELECTOR_PAIR> pseudoSelectors;
    explicit CompoundSelector(optional<TypeSelector> typeSelector = nullopt, vector<SUBCLASS_SELECTOR> subclassSelectors = {}, const vector<PSEUDO_SELECTOR_PAIR>& pseudoSelectors = {})
        : typeSelector(move(typeSelector)),
        subclassSelectors(move(subclassSelectors)),
        pseudoSelectors(move(pseudoSelectors))
    {};
};

// <compound-selector> [ <combinator>? <compound-selector> ]*
#define COMPLEX_SELECTOR_PAIR pair<Combinator, CompoundSelector>
#define COMPLEX_SELECTOR vector<COMPLEX_SELECTOR_PAIR>

// <combinator>? <complex-selector>
struct RelativeSelector {
    Combinator combinator;
    COMPLEX_SELECTOR selector;
    RelativeSelector(Combinator combinator, COMPLEX_SELECTOR selector)
        : selector(move(selector)),
        combinator(move(combinator))
    {};
};

#define SELECTOR_LIST vector<COMPLEX_SELECTOR>
#define COMPLEX_SELECTOR_LIST vector<COMPLEX_SELECTOR>
#define COMPOUND_SELECTOR_LIST vector<CompoundSelector>
#define SIMPLE_SELECTOR_LIST vector<SIMPLE_SELECTOR>
#define RELATIVE_SELECTOR_LIST vector<RelativeSelector>