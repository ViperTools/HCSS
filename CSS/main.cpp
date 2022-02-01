#include "Lexer/Lexer.hpp"
#include "Parser/Macros.hpp"
#include "Parser/Parser.hpp"
#include "Transpiler/Transpiler.hpp"
#include "Util/Logger/logger.hpp"
#include <iostream>
#include <string>

int main() {
	Lexer lexer(R"(/workspace/HCSS/CSS/test.hcss)");    
	auto lexResult = Logger::call<vector<Token>>(std::bind(&Lexer::lex, &lexer));
    lexResult.output("LEXING");

    BaseParser parser(lexResult.result);
    auto parseResult = Logger::call<vector<SYNTAX_NODE>>(std::bind(&BaseParser::parse, &parser));
    parseResult.output("PARSING");

    Transpiler transpiler;
    auto transpile = std::bind(static_cast<void (Transpiler::*)(const vector<SYNTAX_NODE>& p) const> (&Transpiler::visit), &transpiler, parseResult.result);
    Logger::call("TRANSPILING", transpile);
    
    std::wofstream compiled(R"(/workspace/HCSS/CSS/test.css)", std::ios::out);
    compiled << transpiler.getSource();
    Logger::lap();
	return 0;
}