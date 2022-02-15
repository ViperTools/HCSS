// Improve consuming at rules. Possibly handle mixins in the transpiler. Add function calls for mixins and handle arguments

#include "BaseParser.hpp"
#include "ComponentValueParser.hpp"
#include "Grammar/Selector.hpp"
#include "SelectorParser.hpp"
#include "StyleBlockParser.hpp"
#include "Types.hpp"

#include <iostream>
#include <iterator>
#include <variant>

#pragma region Scope

/**
 * @brief Finds a previously defined at-rule. Checks parent scopes recursively.
 *
 * @param name The name of the at-rule
 * @return vector<ComponentValue>* Returns a pointer to the at-rule if successful.
 * @return nullptr Otherwise returns null pointer.
 */

vector<ComponentValue>* Scope::findAtRule(wstring name) {
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

vector<ComponentValue>* Scope::findMixin(wstring name) {
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

vector<ComponentValue>* Scope::findVariable(wstring name) {
    if (variables.contains(name)) {
        return &variables[name];
    }
    else if (parent) {
        return parent -> findVariable(name);
    }
    return nullptr;
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
                if (sel.subclassSelectors.size() > 0) {
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
                auto rule = consumeAtRule();
                if (rule) {
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
                    consumeVariable();
                    return { };
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
        skipws();
        // Add mixin functions
        if (check(IDENT)) {
            auto name = consume(IDENT, "Expected identifier");
            skipws();
            if (check(LEFT_BRACE)) {
                scope.mixins[name.lexeme] = consumeSimpleBlock().value;
            }
            else {
                SYNTAX_ERROR("Expected opening brace", nullopt);
            }
        }
        else if (check(FUNCTION)) {
            auto func = consumeFunctionDefinition();
            /* vector<ComponentValue> value = { func };
            skipws();
            if (!check(LEFT_BRACE)) {
                SYNTAX_ERROR("Expected opening brace", nullopt);
            }
            SimpleBlock block = consumeSimpleBlock();
            std::move(block.value.begin(), block.value.end(), std::back_inserter(value));
            scope.mixins[func.name.lexeme] = value; */
        }
        return nullopt;
    }
    else if (wstrcompi(at.lexeme, L"include")) {
        vector<wstring> include;
        while (!values.empty()) {
            if (check<FunctionDefinition>()) {
                auto f = consume<FunctionDefinition>();
                // Handle function
                if (auto vec = scope.findMixin(f.name.lexeme)) {
                    
                }
            }
            else {
                auto t = consume<Token>();
                if (t.type == SEMICOLON) {
                    break;
                }
                else if (t.type == IDENT) {
                    include.push_back(t.lexeme);
                }
                else if (t.type != WHITESPACE && t.type != COMMA) {
                    SYNTAX_ERROR("Expected identifier or comma", t);
                }
            }
        }
        for (int i = include.size() - 1; i >= 0; i--) {
            if (auto mixin = scope.findMixin(include[i])) {
                values.insert(values.begin(), mixin -> begin(), mixin -> end());
            }
        }
        return nullopt;
    }
    skipws();
    if (check('=') && !wstrcompi(at.lexeme, L"media")) {
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
                    rule.block = *block;
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

FunctionDefinition BaseParser::consumeFunctionDefinition() {
    FunctionDefinition f(consume(FUNCTION, "Expected function"));
    while (auto t = peek<Token>()) {
        switch (t -> type) {
            case WHITESPACE: values.pop_front(); break;
            case T_EOF: case RIGHT_PAREN: {
                values.pop_front();
                return f;
            }
            case COMMA: {
                values.pop_front();
                skipws();
                Token dollar = consume(DELIM, "Expected $");
                if (dollar.lexeme[0] != L'$') {
                    SYNTAX_ERROR("Expected $", dollar);
                }
                Token name = consume(IDENT, "Expected identifier");
                skipws();
                f.parameters[name.lexeme] = {};
                if (check('=')) {
                    values.pop_front();
                    while (!check(',')) {
                        consumeComponentValue(f.parameters[name.lexeme]);
                    }
                }
                break;
            }
            default: {
                SYNTAX_ERROR("Expected ) or parameter", *t);
            }
        }
    }
    return f;
}

FunctionCall BaseParser::consumeFunctionCall() {
    FunctionCall f(consume(FUNCTION, "Expected function"));
    while (auto t = peek<Token>()) {
        switch (t -> type) {
            case T_EOF: case RIGHT_PAREN: {
                values.pop_front();
                return f;
            }
            case COMMA: {
                consumeComponentValue(f.value);
                skipws();
                break;
            }
            default: {
                consumeComponentValue(f.value);
                break;
            }
        }
    }
    return f;
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
    SimpleBlock block(consume<Token>());
    TokenType m = mirror(block.open.type);
    while (!values.empty()) {
        if (auto t = peek<Token>()) {
            if (t -> type == m) {
                block.close = *t;
                values.pop_front();
                return block;
            }
            else if (t -> type == T_EOF) {
                return block;
            }
            else {
                consumeComponentValue(block.value);
            }
        }
        else {
            consumeComponentValue(block.value);
        }
    }
    return block;
}

/**
 * @brief Consumes a value list for things like variables and custom media queries
 *
 * @return vector<ComponentValue> The list of consumed values
 */

vector<ComponentValue> BaseParser::consumeValueList() {
    skipws();
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
 */

void BaseParser::consumeVariable() {
    values.pop_front();
    Token name = consume(IDENT, "Expected identifier");
    skipws();
    if (check('=')) {
        values.pop_front();
        scope.variables[name.lexeme] = consumeValueList();
    }
    else if (auto var = scope.findVariable(name.lexeme)) {
        values.insert(values.begin(), var -> begin(), var -> end());
    }
    else {
        SYNTAX_ERROR("The variable '" + string(name.lexeme.begin(), name.lexeme.end()) + "' was not declared.", name);
    }
}

#pragma endregion