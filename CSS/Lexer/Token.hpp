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
		int column;
		map<string, wstring> flags;
		explicit Token(TokenType type, const wstring& lexeme = {}, int column = -1, int line = -1)
			: type(type),
			lexeme(lexeme),
			column(column - (int) lexeme.length()),
			line(line)
		{};
};