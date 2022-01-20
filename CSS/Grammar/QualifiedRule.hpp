#pragma once

#include "../SyntaxNode.hpp"
#include "SimpleBlock.hpp"
#include <vector>
#include <memory>
using std::vector;
using std::unique_ptr;

class QualifiedRule : public SyntaxNode {
    public:
        vector<COMPONENT_VALUE> components;
        SyntaxNode* block;
        QualifiedRule(vector<COMPONENT_VALUE> components, SyntaxNode* block = NULL)
            : components(components),
            block(block)
        {};
        void accept(SyntaxNodeVisitor visitor) override {
            // visitor.VisitQualifiedRule(*this);
        };
};