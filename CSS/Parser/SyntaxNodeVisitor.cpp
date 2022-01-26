#include "SyntaxNodeVisitor.hpp"
#include <iostream>

void SyntaxNodeVisitor::Visit(std::vector<SyntaxNode> nodes) {
    for (SyntaxNode node : nodes) {
        std::visit([this](auto n) { return this->Visit(n); }, node);
    }
}

void SyntaxNodeVisitor::Visit(AtRule rule) const {
    // std::cout << "AtRule" << std::endl;
}

void SyntaxNodeVisitor::Visit(Function function) const {
    // std::cout << "Function" << std::endl;
}

void SyntaxNodeVisitor::Visit(QualifiedRule rule) const {
    // std::cout << "QualifiedRule" << std::endl;
}

void SyntaxNodeVisitor::Visit(SimpleBlock block) const {
    // std::cout << "SimpleBlock" << std::endl;
}