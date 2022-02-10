#include "StyleBlockParser.hpp"
#include "SelectorParser.hpp"
#include "DeclarationParser.hpp"

#include <iostream>

STYLE_BLOCK StyleBlockParser::parse() {
    STYLE_BLOCK block;
    while (auto t = peek<Token>()) {
        switch (t -> type) {
            case WHITESPACE: case SEMICOLON: values.pop_front(); break;
            case T_EOF: {
                values.pop_front();
                return block;
            }
            case AT_KEYWORD: {
                block.emplace_back(consumeAtRule());
                break;
            }
            case IDENT: {
                vector<COMPONENT_VALUE> temp = { std::move(*t) };
                values.pop_front();
                while (!values.empty()) {
                    auto tok = peek<Token>();
                    if (tok && tok -> type == SEMICOLON) {
                        values.pop_front();
                        break;
                    }
                    consumeComponentValue(temp);
                }
                block.emplace_back(DeclarationParser(temp).parse());
                break;
            }
            case DELIM: {
                if (t -> lexeme[0] == '&') {
                    values.pop_front();
                    QualifiedRule rule = consumeQualifiedRule();
                    if (rule.block) {
                        block.emplace_back(StyleRule(SelectorParser(rule.prelude).parse(), StyleBlockParser(rule.block -> value).parse()));
                    }
                    else {
                        std::cout << "no block" << std::endl;
                        block.emplace_back(StyleRule(SelectorParser(rule.prelude).parse()));
                    }
                }
                break;
            }
            default: {
                while (!values.empty()) {
                    auto tok = peek<Token>();
                    if (tok && tok -> type == SEMICOLON) {
                        values.pop_front();
                        break;
                    }
                    consumeComponentValue();
                }
                break;
            }
        }
    }
    return block;
}