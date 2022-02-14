#pragma once

#include "ComponentValueParser.hpp"
#include "Types.hpp"
#include <map>

class BaseParser : public ComponentValueParser {
    public:
        vector<SyntaxNode> parse();
        using ComponentValueParser::ComponentValueParser;
    protected:
        vector<SyntaxNode> rules;
        std::map<wstring, vector<ComponentValue>> variables;
        std::map<wstring, vector<ComponentValue>> atRules;
        std::map<wstring, vector<ComponentValue>> mixins;
        bool top = true;
        optional<AtRule> consumeAtRule();
        Function consumeFunction();
        QualifiedRule consumeQualifiedRule();
        SimpleBlock consumeSimpleBlock();
        ComponentValue consumeComponentValue();
        void consumeComponentValue(vector<ComponentValue>& vec);
        vector<SyntaxNode> consumeRulesList();
        vector<ComponentValue> consumeValueList();
        void consumeVariable();
};