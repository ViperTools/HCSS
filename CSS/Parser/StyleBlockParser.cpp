#include "StyleBlockParser.hpp"
#include "BaseParser.hpp"
#include "ComponentValueParser.hpp"
#include "Grammar/StyleBlock.hpp"
#include "SelectorParser.hpp"
#include <iostream>

StyleBlock StyleBlockParser::parse() {
    StyleBlock block;
    while (auto t = peek<Token>()) {
        switch (t -> type) {
            case WHITESPACE: case SEMICOLON: values.pop_front(); break;
            case T_EOF: {
                values.pop_front();
                return block;
            }
            case AT_KEYWORD: {
                auto rule = consumeAtRule();
                if (rule) {
                    block.emplace_back(*rule);
                }
                break;
            }
            case IDENT: {
                block.emplace_back(consumeDeclaration());
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

Declaration StyleBlockParser::consumeDeclaration() {
    Token name = consume(IDENT, "Expected identifier"); skipws();
    Token colon = consume(COLON, "Expected colon"); skipws();
    Declaration dec(name, colon);
    while (!values.empty()) {
        auto tok = peek<Token>();
        if (tok && tok -> type == SEMICOLON) {
            values.pop_front();
            break;
        }
        consumeComponentValue(dec.value);
    }
    if (dec.value.size() > 1) {
        if (auto t1 = std::get_if<Token>(&dec.value.back())) {
            if (auto t2 = std::get_if<Token>(&dec.value[dec.value.size() - 2])) {
                if (t1 -> type == DELIM && t1 -> lexeme[0] == L'!' && t2 -> type == IDENT && wstrcompi(t2 -> lexeme, L"important")) {
                    dec.value.pop_back();
                    dec.value.pop_back();
                }
            }
        }
    }
    // Remove all whitespace at end
    for (int i = dec.value.size() - 1; i >= 0; i--) {
        if (auto t = std::get_if<Token>(&dec.value[i])) {
            if (t -> type == WHITESPACE) {
                dec.value.erase(dec.value.begin() + i);
            }
            else {
                break;
            }
        }
    }
    return dec;
}