#pragma once

#include "../SyntaxNode.hpp"
class Function;
#include <vector>
using std::vector;

class SimpleBlock : public SyntaxNode {
    public:
        Token open;
        vector<COMPONENT_VALUE> components;
        SimpleBlock(Token open, vector<COMPONENT_VALUE> components)
            : open(open),
            components(components)
        {};
        void accept(SyntaxNodeVisitor visitor) override {
            // visitor.VisitSimpleBlock(*this);
        };
};