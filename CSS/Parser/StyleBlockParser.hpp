#pragma once

#include "BaseParser.hpp"

class StyleBlockParser : public BaseParser {
    public:
        StyleBlock parse();
        using BaseParser::BaseParser;
    private:
        StyleBlock block;
        Declaration consumeDeclaration();
        optional<AtRule> consumeAtRule();
};