#pragma once
class AtRule;
class Function;
class QualifiedRule;
class SimpleBlock;

class SyntaxNodeVisitor {
    public:
        void VisitAtRule(AtRule rule);
        void VisitFunction(Function function);
        void VisitQualifiedRule(QualifiedRule rule);
        void VisitSimpleBlock(SimpleBlock block);
};