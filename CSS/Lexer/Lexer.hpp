#pragma once

#include "Token.hpp"
#include "TokenType.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <tuple>
using std::string;
using std::vector;
using std::tuple;
using std::wstring;

#define COLUMN ((int) reader.tellg() - posOffset)

class Lexer {
	public:
		vector<Token> tokens;
		vector<Token> lex();
		explicit Lexer(const string& path)
			: reader(path)
		{};
	private:
		std::ifstream reader;
		int posOffset = 0, line = 1;
        wchar_t current{};
		void addToken(TokenType type);
		void consumeWhitespace();
        wchar_t consumeEscapedCodePoint();
		void consumeString();
		wstring consumeIdent();
		void consumeHash();
		bool isIdentSequence();
		tuple<wstring, wstring> consumeNumber();
		void consumeNumericToken();
		void consumeIdentLike();
		void consumeUrl();
		void consumeBadUrl();
		void skipSpace();
};