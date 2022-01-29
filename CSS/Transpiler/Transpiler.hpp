#pragma once
#include "../Parser/SyntaxNodeVisitor.hpp"
#include <string>
using std::wstring;

class Transpiler : public SyntaxNodeVisitor {
    public:
        void visit(const std::vector<SYNTAX_NODE>& nodes) const override;
        void visit(AtRule rule) const override;
        void visit(Function function) const override;
        void visit(QualifiedRule rule) const override;
        void visit(SimpleBlock block) const override;
        void visit(StyleRule rule) const override;
        void visit(std::monostate) const {};
    private:
        wstring stringify(vector<COMPONENT_VALUE> list) const;
        wstring stringify(COMBINATOR comb) const;
        wstring stringify(pair<optional<Token>, Token>& prefix) const;
        wstring stringify(WQ_NAME ts) const;
        wstring stringify(Token& sel) const;
        wstring stringify(pair<Token, Token>& pair) const;
        wstring stringify(AtRule& rule) const;
        wstring stringify(SimpleBlock& block) const;
        wstring stringify(QualifiedRule& rule) const;
        wstring stringify(Function& f) const;
        wstring stringify(StyleRule& rule) const;
        wstring stringify(Declaration& decl) const;
        wstring stringify(const PseudoClassSelector& sel) const;
        wstring stringify(AttributeSelector sel) const;
        wstring stringify(CompoundSelector sel) const;
        wstring stringify(COMPLEX_SELECTOR_LIST list) const;
};