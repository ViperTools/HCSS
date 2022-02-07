#include "Transpiler.hpp"
#include <iostream>
#include <string>
#include <variant>
using std::wstring;
using std::pair;

void Transpiler::visit(const std::vector<SYNTAX_NODE>& nodes) const {
    for (SYNTAX_NODE node : nodes) {
        std::visit([this](auto n) { return this->visit(n); }, node);
    }
}

void Transpiler::visit(AtRule rule) const {
    source += stringify(rule) + L' ';
}

void Transpiler::visit(Function function) const {
    source += stringify(function) + L' ';
}

void Transpiler::visit(QualifiedRule rule) const {
    source += stringify(rule) + L' ';
}

void Transpiler::visit(SimpleBlock block) const {
    source += stringify(block) + L' ';
}

void Transpiler::visit(StyleRule rule) const {
    source += stringify(rule) + L' ';
}

#define VSTRINGIFY(v) (!std::holds_alternative<std::monostate>(v) ? std::visit([this](auto n) -> wstring { return this -> stringify(n); }, v) : L"")
#define OSTRINGIFY(v) ((v) ? stringify(*(v)) : L"")
#define OSTRINGIFY_OR(v, d) ((v).has_value() ? stringify((v).value()) : (d))

wstring Transpiler::stringify(vector<COMPONENT_VALUE> list) const {
    wstring s;
    for (COMPONENT_VALUE cv : list) {
        s += VSTRINGIFY(cv);
    }
    return s;
}

wstring Transpiler::stringify(COMBINATOR comb) const {
    return VSTRINGIFY(comb);
}

wstring Transpiler::stringify(pair<optional<Token>, Token>& prefix) const {
    return OSTRINGIFY(prefix.first) + prefix.second.lexeme;
}

wstring Transpiler::stringify(PseudoElementSelector sel) const {
    return sel.colon.lexeme + stringify(sel.selector);
}

wstring Transpiler::stringify(ClassSelector sel) const {
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
    return L'@' + rule.name.lexeme + stringify(rule.prelude) + OSTRINGIFY_OR(rule.block, L";");
}

wstring Transpiler::stringify(SimpleBlock& block) const {
    return block.open.lexeme + stringify(block.value) + OSTRINGIFY(block.close);
}

wstring Transpiler::stringify(QualifiedRule& rule) const {
    return stringify(rule.prelude) + OSTRINGIFY(rule.block);
}

wstring Transpiler::stringify(Function& f) const {
    return f.name.lexeme + L'(' + stringify(f.value) + L')';
}

wstring Transpiler::stringify(StyleRule& rule, optional<wstring> nestSel) const {
    wstring sel = stringify(rule.selectors);
    if (nestSel) sel = *nestSel + sel;
    wstring s = sel + (rule.block.size() > 0 ? L"{" : L"");
    wstring nest;
    for (STYLE_BLOCK_VARIANT val : rule.block) {
        if (auto r = std::get_if<StyleRule>(&val)) {
            nest += stringify(*r, L":is(" + sel + L")");
        }
        else {
            s += VSTRINGIFY(val);
        }
    }
    return s + (rule.block.size() > 0 ? L"}" : L"") + nest;
}

wstring Transpiler::stringify(Declaration &decl) const {
    return decl.name.lexeme + decl.colon.lexeme + stringify(decl.value) + (decl.important ? L" !important" : L"") + L';';
}

wstring Transpiler::stringify(const PseudoClassSelector& sel) const {
    wstring s = sel.colon.lexeme + sel.tok.lexeme;
    if (sel.closeParen.has_value()) {
        s += L'(';
    }
    return s + OSTRINGIFY(sel.anyValue) + OSTRINGIFY(sel.closeParen);
}

wstring Transpiler::stringify(AttributeSelector sel) const {
    return sel.openBracket.lexeme + stringify(sel.name) + OSTRINGIFY(sel.matcher) + OSTRINGIFY(sel.tok) + OSTRINGIFY(sel.modifier) + sel.closeBracket.lexeme;
}

wstring Transpiler::stringify(CompoundSelector sel) const {
    wstring s = OSTRINGIFY(sel.typeSelector);
    for (SUBCLASS_SELECTOR ss : sel.subclassSelectors) {
        s += VSTRINGIFY(ss);
    }
    for (PSEUDO_SELECTOR_PAIR psp : sel.pseudoSelectors) {
        s += stringify(psp.first);
        for (const PseudoClassSelector& pcs : psp.second) {
            s += stringify(pcs);
        }
    }
    return s;
}

wstring Transpiler::stringify(COMPLEX_SELECTOR_LIST list) const {
    wstring s;
    for (int i = 0; i < list.size(); i++) {
        COMPLEX_SELECTOR sel = list[i];
        for (int j = 0; j < sel.size(); j++) {
            COMPLEX_SELECTOR_PAIR pair = sel[j];
            s += stringify(pair.first) + stringify(pair.second);
            if (j < sel.size() - 1) s += L' ';
        }
        if (i < list.size() - 1) s += L',';
    }
    return s;
}