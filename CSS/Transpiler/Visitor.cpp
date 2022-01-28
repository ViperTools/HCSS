#include "Visitor.hpp"
#include <iostream>

void Visitor::visit(const std::vector<SYNTAX_NODE>& nodes) const {
    for (SYNTAX_NODE node : nodes) {
        std::visit([this](auto n) { return this->visit(n); }, node);
    }
}

void Visitor::visit(AtRule rule) const {
    std::cout << "AtRule" << std::endl;
}

void Visitor::visit(Function function) const {
    std::cout << "Function" << std::endl;
}

void Visitor::visit(QualifiedRule rule) const {
    std::cout << "QualifiedRule" << std::endl;
}

void Visitor::visit(SimpleBlock block) const {
    std::cout << "SimpleBlock" << std::endl;
}

void Visitor::visit(StyleRule rule) const {
    std::cout << "StyleRule" << std::endl;
}