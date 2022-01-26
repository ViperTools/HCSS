#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"
#include "Parser/SyntaxNodeVisitor.hpp"
#include <iostream>

int main() {
	Lexer lexer("CSS/test.css");
	vector<Token> tokens = lexer.Lex();
	StyleSheetParser parser(tokens);
	vector<SyntaxNode> nodes = parser.Parse();
	SyntaxNodeVisitor visitor;
	visitor.Visit(nodes);
	return 0;
}