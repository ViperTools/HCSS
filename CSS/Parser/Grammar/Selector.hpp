#pragma once

#include "../ParserMacros.hpp"
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
#define NsPrefix pair<optional<Token>, Token>

// <ns-prefix>? <ident-token>
#define WqName pair<optional<NsPrefix>, Token>

// [ '~' | '|' | '^' | '$' | '*' ]? '='
#define AttrMatcher pair<optional<Token>, Token>

// i
#define AttrModifier Token

// '>' | '+' | '~' | [ '|' '|' ]
#define Combinator variant<monostate, Token, pair<Token, Token>>

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
            : openBracket(openBracket),
            name(name),
            closeBracket(closeBracket)
        {};
        AttributeSelector(Token openBracket, WqName name, AttrMatcher matcher, Token tok, optional<Token> modifier, Token closeBracket)
            : openBracket(openBracket),
            name(name),
            matcher(matcher),
            tok(tok),
            modifier(modifier),
            closeBracket(closeBracket)
        {};
};

// <wq-name> | <ns-prefix>? '*'
#define TypeSelector WqName

// <hash-token>
#define IdSelector Token

// '.' <ident-token>
#define ClassSelector pair<Token, Token>

// <id-selector> | <class-selector> | <attribute-selector> | <pseudo-class-selector>
#define SubclassSelector variant<monostate, IdSelector, ClassSelector, AttributeSelector, PseudoClassSelector>

// <type-selector> | <subclass-selector>
#define SimpleSelector variant<TypeSelector, SubclassSelector>

// ':' <pseudo-class-selector>
#define PseudoElementSelector pair<Token, PseudoClassSelector>

/*
':' <ident-token> |
    ':' <function-token> <any-value> ')'
*/

class PseudoClassSelector {
    public:
        Token colon;
        Token tok;
        optional<vector<Token>> anyValue;
        optional<Token> closeParen;
        PseudoClassSelector(Token colon, Token tok, optional<vector<Token>> anyValue = nullopt, optional<Token> closeParen = nullopt)
            : colon(colon),
            tok(tok),
            anyValue(anyValue),
            closeParen(closeParen)
        {};
};

/*
[ <type-selector>? <subclass-selector>*
    [ <pseudo-element-selector> <pseudo-class-selector>* ]* ]!
*/
#define PseudoSelectorPair pair<PseudoElementSelector, vector<PseudoClassSelector>>
class CompoundSelector {
    public:
        optional<TypeSelector> typeSelector;
        vector<SubclassSelector> subclassSelectors;
        vector<PseudoSelectorPair> pseudoSelectors;
        CompoundSelector(optional<TypeSelector> typeSelector = nullopt, vector<SubclassSelector> subclassSelectors = {}, vector<PseudoSelectorPair> = {})
            : typeSelector(typeSelector),
            subclassSelectors(subclassSelectors),
            pseudoSelectors(pseudoSelectors)
        {};
};

// <compound-selector> [ <combinator>? <compound-selector> ]*
#define ComplexSelectorPair pair<Combinator, CompoundSelector>
class ComplexSelector {
    public:
        vector<ComplexSelectorPair> selectors;
        ComplexSelector(vector<ComplexSelectorPair> selectors = {})
            : selectors(selectors)
        {};
};

// <combinator>? <complex-selector>
class RelativeSelector {
    public:
        Combinator combinator;
        ComplexSelector selector;
        RelativeSelector(Combinator combinator, ComplexSelector selector)
            : selector(selector),
            combinator(combinator)
        {};
};

#define SelectorList vector<ComplexSelector>
#define ComplexSelectorList vector<ComplexSelector>
#define CompoundSelectorList vector<CompoundSelector>
#define SimpleSelectorList vector<SimpleSelector>
#define RelativeSelectorList vector<RelativeSelector>