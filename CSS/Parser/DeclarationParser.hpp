#pragma once

#include "BaseParser.hpp"

class DeclarationParser : public BaseParser {
    public:
        Declaration parse();
        using BaseParser::BaseParser;
};