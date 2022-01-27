#pragma once

#include "TokenType.hpp"
#include <string>
using std::string;
#include <map>
using std::map;
using std::wstring;

class Token {
	public:
		TokenType type;
		wstring lexeme;
		int line;
		int position;
		map<string, wstring> flags;
		explicit Token(TokenType type, const wstring& lexeme = {}, int position = -1, int line = -1)
			: type(type),
			lexeme(lexeme),
			position(position - lexeme.size()),
			line(line)
		{};
};