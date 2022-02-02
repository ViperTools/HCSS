#include "Lexer/Lexer.hpp"
#include "Parser/Macros.hpp"
#include "Parser/Parser.hpp"
#include "Transpiler/Transpiler.hpp"
#include "Util/Logger/Task.hpp"
#include <iostream>
#include <string>

int main() {
    Task task("PARSER");
	auto lexResult = task.call<vector<Token>>("LEXING", [] () -> auto { return Lexer(R"(C:\Users\User\HCSS\CSS\test.hcss)").lex(); });
    auto parseResult = task.call<vector<SYNTAX_NODE>>("PARSING", [=] () -> auto { return BaseParser(lexResult.result).parse(); });
    Transpiler transpiler;
    task.call("TRANSPILING", [=] { transpiler.visit(parseResult.result); });
    task.log();

    std::wofstream compiled(R"(C:\Users\User\HCSS\CSS\test.css)", std::ios::out);
    compiled << transpiler.getSource();
	return 0;
}