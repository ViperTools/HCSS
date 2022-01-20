#include "Lexer.hpp"
#include "Parser.hpp"
#include <iostream>

int main() {
	Lexer lexer("CSS/test.css");
	vector<Token> tokens = lexer.Lex();
	Parser parser(tokens);
	parser.Parse();
	return 0;
}