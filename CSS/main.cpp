#include "Lexer.hpp"
#include "Parser.hpp"
#include <iostream>

int main() {
	Lexer lexer("CSS/test.css");
	vector<Token> tokens = lexer.Lex();
	Parser parser(tokens);
	vector<snptr> nodes = parser.Parse();
	// SyntaxNodeVisitor visitor;
	return 0;
}