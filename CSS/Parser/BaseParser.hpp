#pragma once

#include "ComponentValueParser.hpp"
#include "Types.hpp"
#include <map>

typedef struct Scope {
    Scope* parent;
    std::map<wstring, vector<ComponentValue>> variables, atRules, mixins;
    vector<ComponentValue>* findVariable(wstring name);
    vector<ComponentValue>* findAtRule(wstring name);
    vector<ComponentValue>* findMixin(wstring name);
} Scope;

class BaseParser : public ComponentValueParser {
    public:
        vector<SyntaxNode> parse();
        using ComponentValueParser::ComponentValueParser;
    protected:
        vector<SyntaxNode> rules;
        Scope scope = {};
        bool top = true;
        optional<AtRule> consumeAtRule();
        FunctionDefinition consumeFunctionDefinition();
        FunctionCall consumeFunctionCall();
        QualifiedRule consumeQualifiedRule();
        SimpleBlock consumeSimpleBlock();
        ComponentValue consumeComponentValue();
        void consumeComponentValue(vector<ComponentValue>& vec);
        vector<SyntaxNode> consumeRulesList();
        vector<ComponentValue> consumeValueList();
        void consumeVariable();
};