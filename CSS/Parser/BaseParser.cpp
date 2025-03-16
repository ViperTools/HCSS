// TODO Improve consuming at rules.
// TODO Improve consuming mixins
// TODO Improve function call and comma separated list consumption. It's pretty ugly right now.
// TODO Improve error messages. Many errors are ambiguous.

#include "BaseParser.hpp"
#include "ComponentValueParser.hpp"
#include "Grammar/Selector.hpp"
#include "SelectorParser.hpp"
#include "StyleBlockParser.hpp"
#include "Types.hpp"

#include <iostream>
#include <iterator>
#include <variant>
#include <queue>

#pragma region Scope

/**
 * @brief Finds a previously defined at-rule. Checks parent scopes recursively.
 *
 * @param name The name of the at-rule
 * @return vector<ComponentValue>* Returns a pointer to the at-rule if successful.
 * @return nullptr Otherwise returns null pointer.
 */

vector<ComponentValue>* Scope::findAtRule(const wstring& name) {
    if (atRules.contains(name)) {
        return &atRules[name];
    }
    else if (parent) {
        return parent -> findAtRule(name);
    }
    return nullptr;
}

/**
 * @brief Finds a previously defined mixin. Checks parent scopes recursively.
 *
 * @param name The name of the mixin
 * @return vector<ComponentValue>* Returns a pointer to the mixin if successful.
 * @return nullptr Otherwise returns null pointer.
 */

Mixin* Scope::findMixin(const wstring& name) {
    if (mixins.contains(name)) {
        return &mixins[name];
    }
    else if (parent) {
        return parent -> findMixin(name);
    }
    return nullptr;
}

/**
 * @brief Finds a previously defined variable. Checks parent scopes recursively.
 *
 * @param name The name of the variable
 * @return vector<ComponentValue>* Returns a pointer to the variable if successful.
 * @return nullptr Otherwise returns null pointer.
 */

vector<ComponentValue>* Scope::findVariable(const wstring& name) {
    if (variables.contains(name)) {
        return &variables[name];
    }
    else if (parent) {
        return parent -> findVariable(name);
    }
    return nullptr;
}

/**
 * @brief Determines whether the variable is in the parameters list.
 *
 * @param name The name of the variable
 */

bool Scope::isParameter(const wstring& name) {
    if (std::count(parameters.begin(), parameters.end(), name)) {
        return true;
    }
    else if (parent) {
        return parent -> isParameter(name);
    }
    return false;
}

#pragma endregion

#pragma region BaseParser

/**
 * @brief Parses a style sheet
 *
 * @return vector<SyntaxNode> A list of parsed SyntaxNodes
 */

vector<SyntaxNode> BaseParser::parse() {
    rules = consumeRulesList();
    top = false;
    for (auto & i : rules) {
        if (auto rule = std::get_if<QualifiedRule>(&i)) {
            ComplexSelectorList selectors = SelectorParser(rule -> prelude).parse();
            // TEST Needs to be changed to accept more than one selector as long as all of them are events
            if (selectors.size() == 1 && selectors.front().size() == 1) {
                CompoundSelector sel = selectors.front().front().second;
                if (!sel.subclassSelectors.empty()) {
                    if (auto pseudo = std::get_if<PseudoClassSelector>(&sel.subclassSelectors.back())) {
                        if (wstrcompi(pseudo -> tok.lexeme, L"click")) {
                            continue;
                        }
                    }
                }
            }
            // Parse style block
            if (rule -> block) {
                StyleBlockParser parser(rule -> block -> value);
                parser.scope.parent = &scope;
                i = StyleRule(selectors, parser.parse());
            }
            else {
                i = StyleRule(selectors);
            }
        }
    }
    return rules;
}

/**
 * @brief Consumes a list of rules
 *
 * @return vector<SyntaxNode> Rules list
 */

vector<SyntaxNode> BaseParser::consumeRulesList() {
    vector<SyntaxNode> list;
    while (auto t = peek<Token>()) {
        switch (t -> type) {
            case WHITESPACE: values.pop_front(); break;
            case T_EOF: return list;
            case CDO: case CDC: {
                if (!top) {
                    list.emplace_back(consumeQualifiedRule());
                }
                break;
            }
            case AT_KEYWORD: {
                if (auto rule = consumeAtRule()) {
                    list.emplace_back(*rule);
                }
                break;
            }
            case DELIM: {
                if (t -> lexeme[0] == '$') {
                    consumeVariable();
                    break;
                }
            }
            default: {
                list.emplace_back(consumeQualifiedRule());
                break;
            }
        }
    }
    return list;
}

/**
 * @brief Consumes a component value.
 *
 * @return monostate if the next component value is a variable, otherwise the next component value
 */

ComponentValue BaseParser::consumeComponentValue() {
    if (auto t = peek<Token>()) {
        switch (t -> type) {
            case LEFT_BRACE: case LEFT_BRACKET: case LEFT_PAREN: {
                return consumeSimpleBlock();
            }
            case FUNCTION: {
                return consumeFunctionCall();
            }
            case DELIM: {
                if (t -> lexeme[0] == '$') {
                    Token temp = std::move(*t);
                    if (consumeVariable())
                        return { };
                    else
                        return temp;
                }
            }
            default: {
                auto temp = std::move(*t);
                values.pop_front();
                return temp;
            }
        }
    }
    return consume();
}

/**
 * @brief Consumes a component value, and if the value is not monostate, pushes it to 'vec'
 *
 * @param vec The vector to push the consumed value to
 */

void BaseParser::consumeComponentValue(vector<ComponentValue>& vec) {
    auto v = consumeComponentValue();
    if (v.index() > 0) {
        vec.emplace_back(v);
    }
}

/**
 * @brief Handles normal at-rules and custom media queries
 *
 * @return optional<AtRule> Returns nullopt if it is a custom media query, otherwise an AtRule
 */

optional<AtRule> BaseParser::consumeAtRule() {
    Token at = consume(AT_KEYWORD, "Expected AT_KEYWORD");
    if (wstrcompi(at.lexeme, L"mixin")) {
        consumeMixin();
    }
    else if (check('=') && !wstrcompi(at.lexeme, L"media")) {
        values.pop_front();
        scope.atRules[at.lexeme] = consumeValueList();
    }
    else if (auto atRule = scope.findAtRule(at.lexeme)) {
        values.insert(values.begin(), atRule -> begin(), atRule -> end());
        values.push_front(Token(AT_KEYWORD, L"media"));
    }
    else {
        AtRule rule(at);
        while (!values.empty()) {
            if (auto t = peek<Token>()) {
                switch (t -> type) {
                    case T_EOF: case SEMICOLON: values.pop_front(); return rule;
                    case LEFT_BRACE: {
                        rule.block = consumeSimpleBlock();
                        return rule;
                    }
                    default: {
                        consumeComponentValue(rule.prelude);
                    }
                }
            }
            else {
                auto block = peek<SimpleBlock>();
                if (block && block -> open.type == LEFT_BRACE) {
                    rule.block = std::move(*block);
                    values.pop_front();
                    return rule;
                }
                else {
                    consumeComponentValue(rule.prelude);
                }
            }
        }
        return rule;
    }
    return nullopt;
}

void BaseParser::consumeMixin() {
    wstring lexeme;
    optional<FunctionDefinition> func;
    if (check(IDENT)) {
        lexeme = consume(IDENT, "Expected identifier").lexeme;
    }
    else if (check(FUNCTION)) {
        func = consumeFunctionDefinition();
        lexeme = func -> name.lexeme;
    }
    if (!check(LEFT_BRACE)) {
        SYNTAX_ERROR("Expected opening brace", nullopt);
    }
    scope.mixins[lexeme] = { func, consumeSimpleBlock().value };
    scope.parameters.clear();
}

vector<vector<ComponentValue>> BaseParser::consumeCommaList() {
    vector<vector<ComponentValue>> value = {{}};
    int parens = 0;
    while (auto t = peek<Token>()) {
        switch (t -> type) {
            case T_EOF: values.pop_front(); break;
            case LEFT_PAREN: {
                value.back().push_back(std::move(*t));
                values.pop_front();
                parens++;
                break;
            }
            case RIGHT_PAREN: {
                if (parens > 0) {
                    parens--;
                    value.back().push_back(std::move(*t));
                    values.pop_front();
                    break;
                }
                else {
                    values.pop_front();
                    return value;
                }
            }
            case COMMA: {
                if (parens == 0) {
                    value.emplace_back();
                }
                else {
                    value.back().push_back(std::move(*t));
                }
                values.pop_front();
                break;
            }
            default: {
                consumeComponentValue(value.back());
            }
        }
    }
    return value;
}

FunctionDefinition BaseParser::consumeFunctionDefinition() {
    FunctionDefinition f(consume(FUNCTION, "Expected function"));
    bool optional = false;
    while (auto t = peek<Token>()) {
        switch (t -> type) {
            case WHITESPACE: values.pop_front(); break;
            case RIGHT_PAREN: {
                values.pop_front();
                return f;
            }
            case COMMA: {
                values.pop_front();
            }
            case DELIM: {
                auto dollar = consume(DELIM, "Expected $");
                if (dollar.lexeme[0] != L'$') {
                    SYNTAX_ERROR("Expected $", dollar);
                }
                Token name = consume(IDENT, "Expected identifier");
                scope.parameters.push_back(name.lexeme);
                vector<ComponentValue> _default;
                if (check('=')) {
                    optional = true;
                    values.pop_front();
                    while (!values.empty() && !check(COMMA) && !check(RIGHT_PAREN)) {
                        consumeComponentValue(_default);
                    }
                }
                else if (optional) {
                    SYNTAX_ERROR("Optional parameters must come last", name);
                }
                f.parameters.emplace_back(name.lexeme, std::move(_default));
                break;
            }
            default: {
                SYNTAX_ERROR("Expected ) or parameter", *t);
            }
        }
    }
    SYNTAX_ERROR("Expected ) to close function definition", f.name);
}

FunctionCall BaseParser::consumeFunctionCall() {
    return { consume(FUNCTION, "Expected function"), consumeCommaList() };
}

QualifiedRule BaseParser::consumeQualifiedRule() {
    QualifiedRule rule;
    while (!values.empty()) {
        if (auto t = peek<Token>()) {
            switch (t -> type) {
                case T_EOF: SYNTAX_ERROR("Qualified rule was not closed. Reached end of file.", nullopt);
                case LEFT_BRACE: {
                    rule.block = consumeSimpleBlock();
                    return rule;
                }
                default: {
                    consumeComponentValue(rule.prelude);
                }
            }
        }
        else {
            auto block = peek<SimpleBlock>();
            if (block && block -> open.type == LEFT_BRACE) {
                rule.block = *block;
                values.pop_front();
                return rule;
            }
            consumeComponentValue(rule.prelude);
        }
    }
    return rule;
}

SimpleBlock BaseParser::consumeSimpleBlock() {
    Scope _scope(scope);
    scope = { &_scope };
    SimpleBlock block(consume<Token>());
    TokenType close = mirror(block.open.type);
    while (!values.empty()) {
        if (auto t = peek<Token>()) {
            if (t -> type == close) {
                block.close = std::move(*t);
                values.pop_front();
                break;
            }
            else if (t -> type == T_EOF) {
                break;
            }
        }
        consumeComponentValue(block.value);
    }
    scope = *scope.parent;
    return block;
}

/**
 * @brief Consumes a value list for things like variables and custom media queries
 *
 * @return vector<ComponentValue> The list of consumed values
 */

vector<ComponentValue> BaseParser::consumeValueList() {
    vector<ComponentValue> val;
    while (!values.empty()) {
        if (auto t = peek<Token>()) {
            if ((t -> type == WHITESPACE && t -> lexeme.find('\n') != string::npos) || t -> type == SEMICOLON) {
                values.pop_front();
                break;
            }
        }
        consumeComponentValue(val);
    }
    return val;
}

/**
 * @brief If it is an assignment, it consumes the name and value and adds it to the variables map
 * @brief Otherwise, it pushes the variable's value to the front of the deque
 *
 * @return Returns whether a variable was consumed or not. Variables are usually only not consumed if it is a parameter.
 */

bool BaseParser::consumeVariable() {
    values.pop_front();
    Token name = consume(IDENT, "Expected identifier");
    if (check(COLON)) {
        values.pop_front();
        scope.variables[name.lexeme] = consumeValueList();
    }
    else if (scope.isParameter(name.lexeme)) {
        values.push_front(name);
        return false;
    }
    else if (auto var = scope.findVariable(name.lexeme)) {
        values.insert(values.begin(), var -> begin(), var -> end());
    }
    else {
        SYNTAX_ERROR("The variable '" + string(name.lexeme.begin(), name.lexeme.end()) + "' was not declared.", name);
    }
    return true;
}

#pragma endregion