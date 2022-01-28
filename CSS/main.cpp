#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"
#include "Parser/SyntaxNodeVisitor.hpp"
#include <iostream>

int main() {
	Lexer lexer(R"(/workspace/CSS Parser/CSS Parser/test.css)");
	vector<Token> tokens = lexer.lex();
	BaseParser parser(tokens);
	vector<SYNTAX_NODE> nodes = parser.parse();
	SyntaxNodeVisitor visitor;
	visitor.Visit(nodes);
	return 0;
}