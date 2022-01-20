#include "Lexer.hpp"
#include <iostream>

void main() {
	Lexer lexer("test.css");
	vector<Token> tokens = lexer.Lex();
	for (Token t : tokens) {
		if (t.type != WHITESPACE) {
			std::cout << t.lexeme << std::endl;
		}
	}
}