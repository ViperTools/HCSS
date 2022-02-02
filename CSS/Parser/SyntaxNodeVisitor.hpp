#pragma once
#include "Macros.hpp"
#include "Grammar/AtRule.hpp"
#include "Grammar/Function.hpp"
#include "Grammar/QualifiedRule.hpp"
#include "Grammar/SimpleBlock.hpp"
#include "Grammar/StyleRule.hpp"
#include "Grammar/Variable.hpp"
#include <vector>

class SyntaxNodeVisitor {
    public:
        virtual void visit(const std::vector<SYNTAX_NODE>& nodes) const = 0;
        virtual void visit(AtRule rule) const = 0;
        virtual void visit(Function function) const = 0;
        virtual void visit(QualifiedRule rule) const = 0;
        virtual void visit(SimpleBlock block) const = 0;
        virtual void visit(StyleRule rule) const = 0;
        virtual void visit(Variable var) const = 0;
        virtual void visit(VariableDeclaration decl) const = 0;
};