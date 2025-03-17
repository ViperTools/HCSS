#pragma once
#include <hcss/parser/types.hpp>
#include <hcss/parser/grammar/atRule.hpp>
#include <hcss/parser/grammar/function.hpp>
#include <hcss/parser/grammar/qualifiedRule.hpp>
#include <hcss/parser/grammar/simpleBlock.hpp>
#include <hcss/parser/grammar/styleRule.hpp>
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