#pragma once

#include "BaseParser.hpp"

class StyleBlockParser : public BaseParser {
    public:
        StyleBlock parse();
        using BaseParser::BaseParser;
};