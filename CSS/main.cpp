#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"
#include "Transpiler/Transpiler.hpp"
#include <iostream>
#include <string>
#include <chrono>
using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::milliseconds;

int main() {

	Lexer lexer(R"(C:\Users\User\CLionProjects\CSS Parser\CSS\test.hcss)");
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
    Transpiler transpiler;
    std::cout << "[DEBUG] TRANSPILING BEGAN" << std::endl;
    auto t5 = high_resolution_clock::now();
    transpiler.visit(nodes);
    auto t6 = high_resolution_clock::now();
    duration<double, std::milli> ms3 = t6 - t5;
    std::cout << "[DEBUG] TRANSPILING COMPLETE: " << ms3.count() << "ms" << std::endl;
    std::cout << "[DEBUG] TOTAL TIME: " << ms.count() + ms2.count() + ms3.count() << "ms" << std::endl;
    std::wofstream compiled(R"(/workspace/CSS/CSS/test.css)", std::ios::out);
    compiled << transpiler.getSource();
	return 0;
}