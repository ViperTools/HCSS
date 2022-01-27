#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"
#include "Parser/SyntaxNodeVisitor.hpp"
#include <iostream>

int main() {
	Lexer lexer(R"(C:\Users\User\CLionProjects\CSS\CSS\test.css)");
	vector<Token> tokens = lexer.lex();
	StyleSheetParser parser(tokens);
	vector<SYNTAX_NODE> nodes = parser.parse();
	SyntaxNodeVisitor visitor;
	visitor.Visit(nodes);
	return 0;
}