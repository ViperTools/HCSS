// TODO Fix consuming a component value by somehow ignoring the returned value if a variable was consumed

#include "BaseParser.hpp"
#include "ComponentValueParser.hpp"
#include "SelectorParser.hpp"
#include "StyleBlockParser.hpp"

#include <iostream>
#include <variant>

/**
 * @brief Parses a base style sheet
 * 
 * @return vector<SYNTAX_NODE> A list of parsed SyntaxNodes
 */

vector<SYNTAX_NODE> BaseParser::parse() {
    rules = consumeRulesList();
    top = false;
    for (auto & i : rules) {
        if (auto rule = std::get_if<QualifiedRule>(&i)) {
            if (rule -> block) {
                i = StyleRule(SelectorParser(rule -> prelude).parse(), StyleBlockParser(rule -> block -> value).parse());
            }
            else {
                i = StyleRule(SelectorParser(rule -> prelude).parse());
            }
        }
    }
    return rules;
}

vector<SYNTAX_NODE> BaseParser::consumeRulesList() {
    vector<SYNTAX_NODE> list;
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
                list.emplace_back(consumeAtRule());
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

COMPONENT_VALUE BaseParser::consumeComponentValue() {
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

void BaseParser::consumeComponentValue(vector<COMPONENT_VALUE>& vec) {
    auto v = consumeComponentValue();
    if (v.index() != 0) {
        vec.emplace_back(v);
    }
}

AtRule BaseParser::consumeAtRule() {
    Token at = consume(AT_KEYWORD, "Expected AT_KEYWORD");
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
            if (block.has_value() && block -> open.type == LEFT_BRACE) {
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

Function BaseParser::consumeFunction() {
    Function f(consume(FUNCTION, "Expected function"));
    while (auto t = peek<Token>()) {
        switch (t -> type) {
            case T_EOF: case RIGHT_PAREN: {
                values.pop_front();
                return f;
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
            if (block.has_value() && block -> open.type == LEFT_BRACE) {
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

void BaseParser::consumeVariable() {
    values.pop_front();
    Token name = consume(IDENT, "Expected identifier");
    IGNORE_WHITESPACE;
    if (check('=')) {
        values.pop_front();
        IGNORE_WHITESPACE;
        vector<COMPONENT_VALUE> val;
        while (!values.empty()) {
            if (auto t = peek<Token>()) {
                if ((t -> type == WHITESPACE && t -> lexeme.find('\n') != string::npos) || t -> type == SEMICOLON) {
                    values.pop_front();
                    break;
                }
            }
            consumeComponentValue(val);
        }
        variables[name.lexeme] = val;
    }
    else if (variables.contains(name.lexeme)) {
        values.insert(values.begin(), variables[name.lexeme].begin(), variables[name.lexeme].end());
    }
    else {
        SYNTAX_ERROR("The variable '" + string(name.lexeme.begin(), name.lexeme.end()) + "' was not declared.", name);
    }
}