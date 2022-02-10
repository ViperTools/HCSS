#pragma once

#include "ComponentValueParser.hpp"
#include "Macros.hpp"
#include <map>

class BaseParser : public ComponentValueParser {
    public:
        vector<SYNTAX_NODE> parse();
        using ComponentValueParser::ComponentValueParser;
    protected:
        vector<SYNTAX_NODE> rules;
        std::map<wstring, vector<COMPONENT_VALUE>> variables;
        bool top = true;
        AtRule consumeAtRule();
        Function consumeFunction();
        QualifiedRule consumeQualifiedRule();
        SimpleBlock consumeSimpleBlock();
        COMPONENT_VALUE consumeComponentValue();
        void consumeComponentValue(vector<COMPONENT_VALUE>& vec);
        vector<SYNTAX_NODE> consumeRulesList();
        void consumeVariable();
};