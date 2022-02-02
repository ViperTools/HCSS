#include "Lexer/Lexer.hpp"
#include "Parser/Macros.hpp"
#include "Parser/Parser.hpp"
#include "Transpiler/Transpiler.hpp"
#include "Util/Logger/Task.hpp"
#include <iostream>
#include <string>

int main() {
    Task task("PARSER");
	auto lexResult = task.call<vector<Token>>("LEXING", [] { return Lexer(R"(/workspace/HCSS/CSS/test.hcss)").lex(); });
    auto parseResult = task.call<vector<SYNTAX_NODE>>("PARSING", [=] { return BaseParser(lexResult.result).parse(); });
    Transpiler transpiler;
    std::wofstream compiled(R"(/workspace/HCSS/CSS/test.css)", std::ios::out);
    compiled << task.call<wstring>("TRANSPILING", [=] { Transpiler t; t.visit(parseResult.result); return t.source; }).result;
    task.log();
	return 0;
}