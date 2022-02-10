#pragma once

#include "BaseParser.hpp"

class StyleBlockParser : public BaseParser {
    public:
        STYLE_BLOCK parse();
        using BaseParser::BaseParser;
};