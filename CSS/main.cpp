#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"
#include "Transpiler/Transpiler.hpp"
#include <iostream>
#include <string>
#include <chrono>

int main() {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

	Lexer lexer(R"(/workspace/CSS/CSS/test.css)");
    std::cout << "[DEBUG] LEXING BEGAN" << std::endl;
    auto t1 = high_resolution_clock::now();
	vector<Token> tokens = lexer.lex();
    auto t2 = high_resolution_clock::now();
    duration<double, std::milli> ms = t2 - t1;
    std::cout << "[DEBUG] LEXING COMPLETE: " << ms.count() << "ms" << std::endl;
    std::cout << "[DEBUG] PARSING BEGAN" << std::endl;
	BaseParser parser(tokens);
    auto t3 = high_resolution_clock::now();
	vector<SYNTAX_NODE> nodes = parser.parse();
    auto t4 = high_resolution_clock::now();
    duration<double, std::milli> ms2 = t4 - t3;
    std::cout << "[DEBUG] PARSING COMPLETE: " << ms2.count() << "ms" << std::endl;
    std::cout << "[DEBUG] TOTAL TIME: " << ms.count() + ms2.count() << "ms" << std::endl;
	Transpiler transpiler;
	transpiler.visit(nodes);
	return 0;
}