#pragma once

#include "TokenType.hpp"
#include <string>
using std::string;
#include <map>
using std::map;

class Token {
	public:
		TokenType type;
		string lexeme;
		int line;
		int position;
		map<string, string> flags;
		Token(TokenType type, string lexeme = "", int position = -1, int line = -1)
			: type(type),
			lexeme(lexeme),
			position(position),
			line(line)
		{};
};