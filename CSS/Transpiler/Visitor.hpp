#pragma once
#include "../Parser/SyntaxNodeVisitor.hpp"

class Visitor : public SyntaxNodeVisitor {
    public:
        void visit(const std::vector<SYNTAX_NODE>& nodes) const override;
        void visit(AtRule rule) const override;
        void visit(Function function) const override;
        void visit(QualifiedRule rule) const override;
        void visit(SimpleBlock block) const override;
        void visit(StyleRule rule) const override;
};