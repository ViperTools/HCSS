#include "DeclarationParser.hpp"
#include "../Util/util.hpp"

Declaration DeclarationParser::parse() {
    Token name = consume(IDENT, "Expected identifier"); IGNORE_WHITESPACE;
    Token colon = consume(COLON, "Expected colon"); IGNORE_WHITESPACE;
    Declaration dec(name, colon);
    while (!values.empty()) {
        consumeComponentValue(dec.value);
    }
    if (dec.value.size() > 1) {
        COMPONENT_VALUE cvSlast = dec.value[dec.value.size() - 2];
        COMPONENT_VALUE cvLast = dec.value.back();
        if (std::holds_alternative<Token>(cvSlast) && std::holds_alternative<Token>(cvLast)) {
            Token tSlast = std::get<Token>(cvSlast);
            Token tLast = std::get<Token>(cvLast);
            if (tSlast.type == DELIM && tSlast.lexeme == L"!" && tLast.type == IDENT && wstrcompi(tLast.lexeme, L"important")) {
                dec.value.pop_back();
                dec.value.pop_back();
            }
        }
    }
    return dec;
}