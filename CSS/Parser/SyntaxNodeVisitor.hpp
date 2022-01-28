#pragma once
#include "Macros.hpp"
#include "Grammar/AtRule.hpp"
#include "Grammar/Function.hpp"
#include "Grammar/QualifiedRule.hpp"
#include "Grammar/SimpleBlock.hpp"
#include "Grammar/StyleRule.hpp"
#include <vector>

class SyntaxNodeVisitor {
    public:
        void Visit(std::vector<SYNTAX_NODE> nodes);
        void Visit(AtRule rule) const;
        void Visit(Function function) const;
        void Visit(QualifiedRule rule) const;
        void Visit(SimpleBlock block) const;
        void Visit(StyleRule rule) const;
};