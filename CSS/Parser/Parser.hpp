#pragma once

#include "../Lexer/Token.hpp"
#include <vector>
#include <variant>
using std::vector;

#include "Grammar/AtRule.hpp"
#include "Grammar/Function.hpp"
#include "Grammar/QualifiedRule.hpp"
#include "Grammar/SimpleBlock.hpp"
#include "Grammar/Selector.hpp"
#include "ParserMacros.hpp"

#define Reconsume idx--
#define Last tokens[idx - 1]

class StyleSheetParser {
    public:
        vector<Token> tokens;
        vector<SyntaxNode> rules;
        vector<SyntaxNode> Parse();
        StyleSheetParser(vector<Token> tokens)
            : tokens(tokens)
        {};
    private:
        int idx = 0;
        bool top = true;
        Token* Peek();
        void Ignore();
        bool Check(TokenType type);
        Token Consume();
        Token Consume(TokenType type, string error);
        AtRule ConsumeAtRule();
        Function ConsumeFunction();
        QualifiedRule ConsumeQualifiedRule();
        SimpleBlock ConsumeSimpleBlock();
        ComponentValue ConsumeComponentValue();
        vector<SyntaxNode> ConsumeRulesList();
};

class SelectorParser {
    public:
        QualifiedRule rule;
        vector<ComplexSelector> Parse();
        SelectorParser(QualifiedRule rule)
            : rule(rule)
        {};
    private:
        int idx = 0;
        ComponentValue* Peek();
        ComponentValue Consume();
        Token* PeekToken();
        Token ConsumeToken();
        Token ConsumeToken(TokenType type, string error);
        bool IsToken();
        bool CheckToken(TokenType type);
        bool CheckToken(string lexeme);
        ComplexSelector ConsumeComplexSelector();
        CompoundSelector ConsumeCompoundSelector();
        AttrMatcher ConsumeAttrMatcher();
        Combinator ConsumeCombinator();
        NsPrefix ConsumeNsPrefix();
        WqName ConsumeWqName();
        TypeSelector ConsumeTypeSelector();
        IdSelector ConsumeIdSelector();
        ClassSelector ConsumeClassSelector();
        SubclassSelector ConsumeSubclassSelector();
        PseudoClassSelector ConsumePseudoClassSelector();
        PseudoElementSelector ConsumePseudoElementSelector();
        AttributeSelector ConsumeAttributeSelector();
        RelativeSelector ConsumeRelativeSelector();
        SimpleSelector ConsumeSimpleSelector();
        vector<Token> ConsumeDeclarationValue(bool any = false);
};