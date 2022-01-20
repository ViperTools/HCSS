#pragma once

#include "../SyntaxNode.hpp"
#include "SimpleBlock.hpp"
#include <vector>
using std::vector;
#include <memory>
using std::unique_ptr;

class AtRule : public SyntaxNode {
    public:
        Token name;
        vector<COMPONENT_VALUE> components;
        SyntaxNode* block;
        AtRule(Token name, vector<COMPONENT_VALUE> components, SyntaxNode* block = NULL)
            : name(name),
            components(components),
            block(block)
        {};
        AtRule(AtRule&& ar)
            : name(ar.name),
            components(ar.components),
            block(std::move(ar.block)) {};
        void accept(SyntaxNodeVisitor visitor) override {
            // visitor.VisitAtRule(*this);
        };
};