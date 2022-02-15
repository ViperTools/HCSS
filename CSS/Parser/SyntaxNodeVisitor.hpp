#pragma once
#include "Types.hpp"
#include "Grammar/AtRule.hpp"
#include "Grammar/Function.hpp"
#include "Grammar/QualifiedRule.hpp"
#include "Grammar/SimpleBlock.hpp"
#include "Grammar/StyleRule.hpp"
#include <vector>
#include <deque>

class SyntaxNodeVisitor {
    public:
        virtual void visit(const std::vector<SyntaxNode>& nodes) const = 0;
        virtual void visit(AtRule rule) const = 0;
        virtual void visit(FunctionCall function) const = 0;
        virtual void visit(QualifiedRule rule) const = 0;
        virtual void visit(SimpleBlock block) const = 0;
        virtual void visit(StyleRule rule) const = 0;
};