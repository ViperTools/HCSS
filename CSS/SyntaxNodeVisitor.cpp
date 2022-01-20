#include "SyntaxNodeVisitor.hpp"

#include "Grammar/AtRule.hpp"
#include "Grammar/Function.hpp"
#include "Grammar/QualifiedRule.hpp"
#include "Grammar/SimpleBlock.hpp"
#include <iostream>

void SyntaxNodeVisitor::VisitAtRule(AtRule rule) {
    std::cout << "AtRule" << std::endl;
}

void SyntaxNodeVisitor::VisitFunction(Function function) {
    std::cout << "Function" << std::endl;
}

void SyntaxNodeVisitor::VisitQualifiedRule(QualifiedRule rule) {
    std::cout << "QualifiedRule" << std::endl;
}

void SyntaxNodeVisitor::VisitSimpleBlock(SimpleBlock block) {
    std::cout << "SimpleBlock" << std::endl;
}