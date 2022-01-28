#pragma once

#include "Errors/SyntaxError.hpp"
#include "../Lexer/Token.hpp"
#include <utility>
#include <vector>
#include <variant>
#include "Grammar/AtRule.hpp"
#include "Grammar/Function.hpp"
#include "Grammar/QualifiedRule.hpp"
#include "Grammar/SimpleBlock.hpp"
#include "Grammar/Selector.hpp"
#include "Grammar/StyleBlock.hpp"
#include "Grammar/StyleRule.hpp"
#include "Macros.hpp"
using std::vector;

#define SYNTAX_ERROR(s, t) throw SyntaxError(s, t, __LINE__, __FILE__)
#define SKIP idx++
#define RECONSUME idx--
#define IGNORE_WHITESPACE if (check(WHITESPACE)) idx++
#define NOT_EOF (idx < values.size() && !check(T_EOF))

class ComponentValueParser {
    public:
        explicit ComponentValueParser(vector<COMPONENT_VALUE> values)
            : values(values)
        {};
        explicit ComponentValueParser(vector<Token> tokens)
        {
            for (Token t : tokens) {
                values.emplace_back(std::move(t));
            }
        };
    protected:
        vector<COMPONENT_VALUE> values;
        int idx = 0;
        template<typename T = COMPONENT_VALUE> T consume();
        Token consume(TokenType type, const string& error);
        template<typename T = COMPONENT_VALUE> optional<T> peek();
        template<typename T = COMPONENT_VALUE> bool check();
        bool check(TokenType type);
        bool check(const wstring& lexeme);
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
        ATTR_MATCHER consumeAttrMatcher();
        COMBINATOR consumeCombinator();
        NS_PREFIX consumeNsPrefix();
        WQ_NAME consumeWqName();
        TYPE_SELECTOR consumeTypeSelector();
        CLASS_SELECTOR consumeClassSelector();
        SUBCLASS_SELECTOR consumeSubclassSelector();
        PseudoClassSelector consumePseudoClassSelector();
        PSEUDO_ELEMENT_SELECTOR consumePseudoElementSelector();
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