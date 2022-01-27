#pragma once

#include "../Lexer/Token.hpp"
#include <utility>
#include <vector>
#include <variant>
#include "Grammar/AtRule.hpp"
#include "Grammar/Function.hpp"
#include "Grammar/QualifiedRule.hpp"
#include "Grammar/SimpleBlock.hpp"
#include "Grammar/Selector.hpp"
#include "ParserMacros.hpp"
using std::vector;

#define SKIP idx++
#define RECONSUME idx--
#define LAST tokens[idx - 1]

class StyleSheetParser {
    public:
        vector<SYNTAX_NODE> parse();
        explicit StyleSheetParser(vector<Token> tokens)
            : tokens(std::move(tokens))
        {};
    private:
        vector<Token> tokens;
        vector<SYNTAX_NODE> rules;
        int idx = 0;
        bool top = true;
        Token* peek();
        bool check(TokenType type);
        Token consume();
        Token consume(TokenType type, const string& error);
        AtRule consumeAtRule();
        Function consumeFunction();
        QualifiedRule consumeQualifiedRule();
        SimpleBlock consumeSimpleBlock();
        COMPONENT_VALUE consumeComponentValue();
        vector<SYNTAX_NODE> consumeRulesList();
};

#define IGNORE_WHITESPACE if (checkToken(WHITESPACE)) idx++

class SelectorParser {
    public:
        vector<ComplexSelector> parse();
        explicit SelectorParser(QualifiedRule rule)
            : rule(std::move(rule))
        {};
    private:
        QualifiedRule rule;
        int idx = 0;
        COMPONENT_VALUE* peek();
        COMPONENT_VALUE consume();
        Token* peekToken();
        Token consumeToken();
        Token consumeToken(TokenType type, const string& error);
        bool isToken();
        bool checkToken(TokenType type);
        bool checkToken(const wstring& lexeme);
        ComplexSelector consumeComplexSelector();
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
        vector<Token> consumeDeclarationValue(bool any = false);
};