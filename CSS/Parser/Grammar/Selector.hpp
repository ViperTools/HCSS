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
#define NS_PREFIX pair<optional<Token>, Token>

// <ns-prefix>? <ident-token>
#define WQ_NAME pair<optional<NS_PREFIX>, Token>

// [ '~' | '|' | '^' | '$' | '*' ]? '='
#define ATTR_MATCHER pair<optional<Token>, Token>

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
        WQ_NAME name;
        optional<ATTR_MATCHER> matcher = nullopt;
        optional<Token> tok = nullopt;
        optional<Token> modifier = nullopt;
        Token closeBracket;
        AttributeSelector(Token openBracket, WQ_NAME name, Token closeBracket)
            : openBracket(std::move(openBracket)),
            name(std::move(name)),
            closeBracket(std::move(closeBracket))
        {};
        AttributeSelector(Token openBracket, WQ_NAME name, ATTR_MATCHER matcher, Token tok, optional<Token> modifier, Token closeBracket)
            : openBracket(std::move(openBracket)),
            name(std::move(name)),
            matcher(matcher),
            tok(tok),
            modifier(std::move(modifier)),
            closeBracket(std::move(closeBracket))
        {};
};

// <wq-name> | <ns-prefix>? '*'
#define TYPE_SELECTOR WQ_NAME

// <hash-token>
#define ID_SELECTOR Token

// '.' <ident-token>
#define CLASS_SELECTOR pair<Token, Token>

// <id-selector> | <class-selector> | <attribute-selector> | <pseudo-class-selector>
#define SUBCLASS_SELECTOR variant<monostate, ID_SELECTOR, CLASS_SELECTOR, AttributeSelector, PseudoClassSelector>

// <type-selector> | <subclass-selector>
#define SIMPLE_SELECTOR variant<TYPE_SELECTOR, SUBCLASS_SELECTOR>

// ':' <pseudo-class-selector>
#define PSEUDO_ELEMENT_SELECTOR pair<Token, PseudoClassSelector>

/*
':' <ident-token> |
    ':' <function-token> <any-value> ')'
*/

class PseudoClassSelector {
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

/*
[ <type-selector>? <subclass-selector>*
    [ <pseudo-element-selector> <pseudo-class-selector>* ]* ]!
*/
#define PSEUDO_SELECTOR_PAIR pair<PSEUDO_ELEMENT_SELECTOR, vector<PseudoClassSelector>>
class CompoundSelector {
    public:
        optional<TYPE_SELECTOR> typeSelector;
        vector<SUBCLASS_SELECTOR> subclassSelectors;
        vector<PSEUDO_SELECTOR_PAIR> pseudoSelectors;
        explicit CompoundSelector(optional<TYPE_SELECTOR> typeSelector = nullopt, vector<SUBCLASS_SELECTOR> subclassSelectors = {}, const vector<PSEUDO_SELECTOR_PAIR>& pseudoSelectors = {})
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