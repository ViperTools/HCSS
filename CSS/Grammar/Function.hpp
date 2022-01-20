#pragma once

#include "../SyntaxNode.hpp"
class SimpleBlock;
#include <vector>
using std::vector;

class Function : public SyntaxNode {
    public:
        Token name;
        vector<COMPONENT_VALUE> components;
        Function(Token name, vector<COMPONENT_VALUE> components)
            : name(name),
            components(components)
        {};
        void accept(SyntaxNodeVisitor visitor) override {
            // visitor.VisitFunction(*this);
        };
};