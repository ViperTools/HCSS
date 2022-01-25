#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"
#include "Parser/SyntaxNodeVisitor.hpp"
#include <iostream>

int main() {
	Lexer lexer("CSS/test.css");
	vector<Token> tokens = lexer.Lex();
	Parser parser(tokens);
	vector<SYNTAX_NODE> nodes = parser.Parse();
	SyntaxNodeVisitor visitor;
	visitor.Visit(nodes);
	return 0;
}