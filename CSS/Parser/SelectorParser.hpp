#pragma once

#include "ComponentValueParser.hpp"

class SelectorParser : public ComponentValueParser {
    public:
        COMPLEX_SELECTOR_LIST parse();
        using ComponentValueParser::ComponentValueParser;
    private:
        COMPLEX_SELECTOR consumeComplexSelector();
        CompoundSelector consumeCompoundSelector();
        AttrMatcher consumeAttrMatcher();
        Combinator consumeCombinator();
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