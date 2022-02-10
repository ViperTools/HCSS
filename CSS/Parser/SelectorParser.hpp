#pragma once

#include "ComponentValueParser.hpp"

class SelectorParser : public ComponentValueParser {
    public:
        ComplexSelectorList parse();
        using ComponentValueParser::ComponentValueParser;
    private:
        ComplexSelector consumeComplexSelector();
        CompoundSelector consumeCompoundSelector();
        AttrMatcher consumeAttrMatcher();
        Combinator consumeCombinator();
        NsPrefix consumeNsPrefix();
        WqName consumeWqName();
        TypeSelector consumeTypeSelector();
        ClassSelector consumeClassSelector();
        SubclassSelector consumeSubclassSelector();
        PseudoClassSelector consumePseudoClassSelector();
        PseudoElementSelector consumePseudoElementSelector();
        AttributeSelector consumeAttributeSelector();
        RelativeSelector consumeRelativeSelector();
        SimpleSelector consumeSimpleSelector();
        vector<ComponentValue> consumeDeclarationValue(bool any = false);
};