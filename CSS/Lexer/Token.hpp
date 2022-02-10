#pragma once

#include "TokenType.hpp"
#include <string>
#include <map>
using std::wstring;
using std::string;

struct Token {
	TokenType type;
	wstring lexeme;
	int line, column;
	std::map<string, wstring> flags;
	explicit Token(TokenType type, const wstring& lexeme = {}, int column = -1, int line = -1)
		: type(type),
		lexeme(lexeme),
		column(column - (int) lexeme.length()),
		line(line)
	{};
};