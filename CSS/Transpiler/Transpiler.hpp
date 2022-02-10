#pragma once
#include "../Parser/SyntaxNodeVisitor.hpp"
#include <string>
#include <map>
using std::wstring;

class Transpiler : public SyntaxNodeVisitor {
    public:
        mutable wstring source;
        void visit(const std::vector<SyntaxNode>& nodes) const override;
        void visit(AtRule rule) const override;
        void visit(Function function) const override;
        void visit(QualifiedRule rule) const override;
        void visit(SimpleBlock block) const override;
        void visit(StyleRule rule) const override;
        void visit(std::monostate) const {};
    private:
        wstring stringify(vector<ComponentValue> list) const;
        wstring stringify(Combinator comb) const;
        wstring stringify(pair<optional<Token>, Token>& prefix) const;
        wstring stringify(PseudoElementSelector sel) const;
        wstring stringify(ClassSelector sel) const;
        wstring stringify(TypeSelector sel) const;
        wstring stringify(AttrMatcher matcher) const;
        wstring stringify(NsPrefix prefix) const;
        wstring stringify(WqName ts) const;
        wstring stringify(Token& sel) const;
        wstring stringify(pair<Token, Token>& pair) const;
        wstring stringify(AtRule& rule) const;
        wstring stringify(SimpleBlock& block) const;
        wstring stringify(QualifiedRule& rule) const;
        wstring stringify(Function& f) const;
        wstring stringify(StyleRule& rule, optional<wstring> nestSel = std::nullopt) const;
        wstring stringify(Declaration& decl) const;
        wstring stringify(const PseudoClassSelector& sel) const;
        wstring stringify(AttributeSelector sel) const;
        wstring stringify(CompoundSelector sel) const;
        wstring stringify(ComplexSelectorList list) const;
};