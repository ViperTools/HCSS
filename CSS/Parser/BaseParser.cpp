// Improve consuming at rules. Possibly handle mixins in the transpiler.

#include "BaseParser.hpp"
#include "ComponentValueParser.hpp"
#include "SelectorParser.hpp"
#include "StyleBlockParser.hpp"
#include "Types.hpp"

#include <iostream>
#include <variant>

/**
 * @brief Parses a base style sheet
 * 
 * @return vector<SyntaxNode> A list of parsed SyntaxNodes
 */

vector<SyntaxNode> BaseParser::parse() {
    rules = consumeRulesList();
    top = false;
    for (auto & i : rules) {
        if (auto rule = std::get_if<QualifiedRule>(&i)) {
            if (rule -> block) {
                StyleBlockParser parser(rule -> block -> value);
                parser.mixins = mixins;
                i = StyleRule(SelectorParser(rule -> prelude).parse(), parser.parse());
            }
            else {
                i = StyleRule(SelectorParser(rule -> prelude).parse());
            }
        }
    }
    return rules;
}

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
                return consumeFunction();
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
        IGNORE_WHITESPACE;
        auto name = consume(IDENT, "Expected identifier");
        IGNORE_WHITESPACE;
        if (check(LEFT_BRACE)) {
            mixins[name.lexeme] = consumeSimpleBlock().value;
        }
        else {
            SYNTAX_ERROR("Expected opening brace", nullopt);
        }
        return nullopt;
    }
    else if (wstrcompi(at.lexeme, L"include")) {
        vector<wstring> include;
        while (!values.empty()) {
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
        for (int i = include.size() - 1; i >= 0; i--) {
            wstring s = include[i];
            if (mixins.contains(s)) {
                values.insert(values.begin(), mixins[s].begin(), mixins[s].end());
            }
        }
        return nullopt;
    }
    IGNORE_WHITESPACE;
    if (check('=') && !wstrcompi(at.lexeme, L"media")) {
        values.pop_front();
        atRules[at.lexeme] = consumeValueList();
    }
    else if (atRules.contains(at.lexeme)) {
        values.insert(values.begin(), atRules[at.lexeme].begin(), atRules[at.lexeme].end());
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

Function BaseParser::consumeFunction() {
    Function f(consume(FUNCTION, "Expected function"));
    while (auto t = peek<Token>()) {
        switch (t -> type) {
            case T_EOF: case RIGHT_PAREN: {
                values.pop_front();
                return f;
            }
            case COMMA: {
                consumeComponentValue(f.value);
                IGNORE_WHITESPACE;
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
    IGNORE_WHITESPACE;
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
    IGNORE_WHITESPACE;
    if (check('=')) {
        values.pop_front();
        variables[name.lexeme] = consumeValueList();
    }
    else if (variables.contains(name.lexeme)) {
        values.insert(values.begin(), variables[name.lexeme].begin(), variables[name.lexeme].end());
    }
    else {
        SYNTAX_ERROR("The variable '" + string(name.lexeme.begin(), name.lexeme.end()) + "' was not declared.", name);
    }
}