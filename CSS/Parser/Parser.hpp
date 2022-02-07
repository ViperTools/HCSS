#pragma once

#include "Errors/SyntaxError.hpp"
#include "../Lexer/Token.hpp"
#include <utility>
#include <vector>
#include <variant>
#include <deque>
#include "Grammar/AtRule.hpp"
#include "Grammar/Function.hpp"
#include "Grammar/QualifiedRule.hpp"
#include "Grammar/SimpleBlock.hpp"
#include "Grammar/Selector.hpp"
#include "Grammar/StyleBlock.hpp"
#include "Grammar/StyleRule.hpp"
#include "Macros.hpp"
using std::vector;
using std::deque;

#define SYNTAX_ERROR(s, t) throw SyntaxError(s, t, __LINE__, __FILE__)
#define IGNORE_WHITESPACE if (check(WHITESPACE)) values.pop_front();

class ComponentValueParser {
    public:
        explicit ComponentValueParser(vector<COMPONENT_VALUE> vec)
        {
            std::move(vec.begin(), vec.end(), std::back_inserter(values));
        };
        explicit ComponentValueParser(deque<COMPONENT_VALUE> values)
            : values(std::move(values))
        {};
        explicit ComponentValueParser(const vector<Token>& tokens)
        {
            std::move(tokens.begin(), tokens.end(), std::back_inserter(values));
        };
    protected:
        deque<COMPONENT_VALUE> values;
        template<typename T = COMPONENT_VALUE> T consume();
        Token consume(TokenType type, const string& error);
        template<typename T = COMPONENT_VALUE> optional<T> peek(int idx = 0);
        template<typename T = COMPONENT_VALUE> bool check();
        bool check(TokenType type, int idx = 0);
        bool check(const wstring& lexeme, int idx = 0);
        bool check(const wchar_t& lexeme, int idx = 0);
};

class BaseParser : public ComponentValueParser {
    public:
        vector<SYNTAX_NODE> parse();
        using ComponentValueParser::ComponentValueParser;
    protected:
        vector<SYNTAX_NODE> rules;
        bool top = true;
        AtRule consumeAtRule();
        Function consumeFunction();
        QualifiedRule consumeQualifiedRule();
        SimpleBlock consumeSimpleBlock();
        COMPONENT_VALUE consumeComponentValue();
        vector<SYNTAX_NODE> consumeRulesList();
};

class SelectorParser : public ComponentValueParser {
    public:
        COMPLEX_SELECTOR_LIST parse();
        using ComponentValueParser::ComponentValueParser;
    private:
        COMPLEX_SELECTOR consumeComplexSelector();
        CompoundSelector consumeCompoundSelector();
        AttrMatcher consumeAttrMatcher();
        COMBINATOR consumeCombinator();
        NsPrefix consumeNsPrefix();
        WqName consumeWqName();
        TypeSelector consumeTypeSelector();
        ClassSelector consumeClassSelector();
        SUBCLASS_SELECTOR consumeSubclassSelector();
        PseudoClassSelector consumePseudoClassSelector();
        PseudoElementSelector consumePseudoElementSelector();
        AttributeSelector consumeAttributeSelector();
        RelativeSelector consumeRelativeSelector();
        SIMPLE_SELECTOR consumeSimpleSelector();
        vector<COMPONENT_VALUE> consumeDeclarationValue(bool any = false);
};

class StyleBlockParser : public BaseParser {
    public:
        STYLE_BLOCK parse();
        using BaseParser::BaseParser;
};

class DeclarationParser : public BaseParser {
    public:
        Declaration parse();
        using BaseParser::BaseParser;
};