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
    // std::cout << "AtRule" << std::endl;
}

void Transpiler::visit(Function function) const {
    // std::cout << "Function" << std::endl;
}

void Transpiler::visit(QualifiedRule rule) const {
    // std::cout << "QualifiedRule" << std::endl;
}

void Transpiler::visit(SimpleBlock block) const {
    // std::cout << "SimpleBlock" << std::endl;
}

void Transpiler::visit(StyleRule rule) const {
    COMPLEX_SELECTOR_LIST selectors = rule.selectors;
    stringify(selectors);
    STYLE_BLOCK block = rule.block;
}

#define VSTRINGIFY(v) (!std::holds_alternative<std::monostate>(v) ? std::visit([this](auto n) -> wstring { return this -> stringify(n); }, v) : L"")
#define OSTRINGIFY(v) (v.has_value() ? stringify(v.value()) : L"")

wstring Transpiler::stringify(COMBINATOR comb) const {
    return VSTRINGIFY(comb);
}

wstring Transpiler::stringify(pair<optional<Token>, Token>& prefix) const {
    return OSTRINGIFY(prefix.first) + prefix.second.lexeme;
}

wstring Transpiler::stringify(WQ_NAME ts) const {
    return OSTRINGIFY(ts.first) + ts.second.lexeme;
}

wstring Transpiler::stringify(Token& t) const {
    switch (t.type) {
        case HASH: return L'#' + t.lexeme;
        default: return t.lexeme;
    }
}

wstring Transpiler::stringify(pair<Token, Token> pair) const {
    return stringify(pair.first) + stringify(pair.second);
}

wstring Transpiler::stringify(PseudoClassSelector sel) const {
    return L"";
}

wstring Transpiler::stringify(AttributeSelector sel) const {
    return stringify(sel.name) + OSTRINGIFY(sel.matcher) + OSTRINGIFY(sel.tok) + OSTRINGIFY(sel.modifier) + sel.closeBracket.lexeme;
}

wstring Transpiler::stringify(CompoundSelector sel) const {
    return OSTRINGIFY(sel.typeSelector);
}

wstring Transpiler::stringify(COMPLEX_SELECTOR_LIST list) const {
    wstring s;
    for (COMPLEX_SELECTOR sel : list) {
        for (COMPLEX_SELECTOR_PAIR pair : sel) {
            s += stringify(pair.first) + stringify(pair.second);
        }
    }
    std::cout << string(s.begin(), s.end()) << std::endl;
    return s;
}