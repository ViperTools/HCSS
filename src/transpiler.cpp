#include <transpiler.hpp>
#include <hcss/parser/parser.hpp>
#include <iostream>
#include <string>
#include <variant>
using std::wstring;
using std::pair;

void Transpiler::visit(const std::vector<SyntaxNode>& nodes) const {
    for (SyntaxNode node : nodes) {
        std::visit([this](auto n) { return this->visit(n); }, node);
    }
}

void Transpiler::visit(AtRule rule) const {
    source += stringify(rule);
}

void Transpiler::visit(FunctionCall function) const {
    source += stringify(function);
}

void Transpiler::visit(QualifiedRule rule) const {
    source += stringify(rule);
}

void Transpiler::visit(SimpleBlock block) const {
    source += stringify(block);
}

void Transpiler::visit(StyleRule rule) const {
    source += stringify(rule);
}

#define VSTRINGIFY(v) (!std::holds_alternative<std::monostate>(v) ? std::visit([this](auto n) -> wstring { return this -> stringify(n); }, v) : L"")
#define OSTRINGIFY(v) ((v) ? stringify(*(v)) : L"")
#define OSTRINGIFY_OR(v, d) ((v) ? stringify((v).value()) : (d))

wstring Transpiler::stringify(const vector<ComponentValue>& list) const {
    wstring s;
    for (ComponentValue cv : list) {
        s += VSTRINGIFY(cv);
    }
    return s;
}

wstring Transpiler::stringify(vector<vector<ComponentValue>> list) const {
    wstring s;
    for (int i = 0; i < list.size(); i++) {
        s += stringify(list[i]);
        if (i < list.size()) s += L',';
    }
    return s;
}

wstring Transpiler::stringify(Combinator comb) const {
    return VSTRINGIFY(comb);
}

wstring Transpiler::stringify(pair<optional<Token>, Token>& prefix) const {
    return OSTRINGIFY(prefix.first) + prefix.second.lexeme;
}

wstring Transpiler::stringify(const PseudoElementSelector& sel) const {
    return sel.colon.lexeme + stringify(sel.selector);
}

wstring Transpiler::stringify(const ClassSelector& sel) const {
    return sel.dot.lexeme + sel.ident.lexeme;
}

wstring Transpiler::stringify(TypeSelector sel) const {
    if (sel.wqName) {
        return stringify(*sel.wqName);
    }
    else {
        return OSTRINGIFY(sel.nsPrefix) + sel.star -> lexeme;
    }
}

wstring Transpiler::stringify(AttrMatcher matcher) const {
    return OSTRINGIFY(matcher.tok) + matcher.eq.lexeme;
}

wstring Transpiler::stringify(NsPrefix prefix) const {
    return OSTRINGIFY(prefix.value) + prefix.bar.lexeme;
}

wstring Transpiler::stringify(WqName ts) const {
    return OSTRINGIFY(ts.prefix) + ts.ident.lexeme;
}

wstring Transpiler::stringify(Token& t) const {
    switch (t.type) {
        case HASH: return L'#' + t.lexeme;
        case STRING: return t.flags["quote"]  + t.lexeme + t.flags["quote"];
        case DIMENSION: return t.lexeme + t.flags["unit"];
        case PERCENTAGE: return t.lexeme + L'%';
        default: return t.lexeme;
    }
}

wstring Transpiler::stringify(pair<Token, Token>& pair) const {
    return pair.first.lexeme + pair.second.lexeme;
}

wstring Transpiler::stringify(AtRule& rule) const {
    source += L'@' + rule.name.lexeme + L' ' + stringify(rule.prelude);
    if (rule.block) {
        source += L'{';
        visit(Parser(rule.block -> value).parse());
        return L"}";
    }
    return L";";
}

wstring Transpiler::stringify(SimpleBlock& block) const {
    return block.open.lexeme + stringify(block.value) + OSTRINGIFY(block.close);
}

wstring Transpiler::stringify(QualifiedRule& rule) const {
    return stringify(rule.prelude) + OSTRINGIFY(rule.block);
}

wstring Transpiler::stringify(FunctionCall& f) const {
    return f.name.lexeme + L'(' + stringify(f.arguments) + L')';
}

wstring Transpiler::stringify(StyleRule& rule, optional<wstring> nestSel) const {
    wstring sel = stringify(rule.selectors);
    if (nestSel) sel = *nestSel + sel;
    wstring s = sel + (!rule.block.empty() ? L"{" : L"");
    wstring nest;
    for (StyleBlockVariant val : rule.block) {
        if (auto r = std::get_if<StyleRule>(&val)) {
            nest += stringify(*r, L":is(" + sel + L")");
        }
        else {
            s += VSTRINGIFY(val);
        }
    }
    return s + (!rule.block.empty() ? L"}" : L"") + nest;
}

wstring Transpiler::stringify(Declaration &decl) const {
    return decl.name.lexeme + decl.colon.lexeme + stringify(decl.value) + (decl.important ? L" !important" : L"") + L';';
}

wstring Transpiler::stringify(const PseudoClassSelector& sel) const {
    wstring s = sel.colon.lexeme + sel.tok.lexeme;
    if (sel.closeParen) {
        s += L'(';
    }
    return s + OSTRINGIFY(sel.anyValue) + OSTRINGIFY(sel.closeParen);
}

wstring Transpiler::stringify(AttributeSelector sel) const {
    return sel.openBracket.lexeme + stringify(sel.name) + OSTRINGIFY(sel.matcher) + OSTRINGIFY(sel.tok) + OSTRINGIFY(sel.modifier) + sel.closeBracket.lexeme;
}

wstring Transpiler::stringify(CompoundSelector sel) const {
    wstring s = OSTRINGIFY(sel.typeSelector);
    for (SubclassSelector ss : sel.subclassSelectors) {
        s += VSTRINGIFY(ss);
    }
    for (const PseudoSelectorPair& psp : sel.pseudoSelectors) {
        s += stringify(psp.first);
        for (const PseudoClassSelector& pcs : psp.second) {
            s += stringify(pcs);
        }
    }
    return s;
}

wstring Transpiler::stringify(ComplexSelectorList list) const {
    wstring s;
    for (int i = 0; i < list.size(); i++) {
        ComplexSelector sel = list[i];
        for (int j = 0; j < sel.size(); j++) {
            s += stringify(sel[j].first) + stringify(sel[j].second);
            if (j < sel.size() - 1) s += L' ';
        }
        if (i < list.size() - 1) s += L',';
    }
    return s;
}