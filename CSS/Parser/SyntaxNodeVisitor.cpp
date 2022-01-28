#include "SyntaxNodeVisitor.hpp"
#include <iostream>

void SyntaxNodeVisitor::Visit(std::vector<SYNTAX_NODE> nodes) {
    for (SYNTAX_NODE node : nodes) {
        std::visit([this](auto n) { return this->Visit(n); }, node);
    }
}

void SyntaxNodeVisitor::Visit(AtRule rule) const {
    std::cout << "AtRule" << std::endl;
}

void SyntaxNodeVisitor::Visit(Function function) const {
    std::cout << "Function" << std::endl;
}

void SyntaxNodeVisitor::Visit(QualifiedRule rule) const {
    std::cout << "QualifiedRule" << std::endl;
}

void SyntaxNodeVisitor::Visit(SimpleBlock block) const {
    std::cout << "SimpleBlock" << std::endl;
}

void SyntaxNodeVisitor::Visit(StyleRule rule) const {
    std::cout << "StyleRule" << std::endl;
}