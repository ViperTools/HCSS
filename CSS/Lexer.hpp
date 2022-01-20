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

class Lexer {
	public:
		vector<Token> tokens;
		vector<Token> Lex();
		Lexer(string path)
			: reader(path)
		{};
	private:
		std::ifstream reader;
		int pos = 0, line = 1;
		char current;
		void AddToken(TokenType type);
		void ConsumeWhitespace();
		char ConsumeEscapedCodePoint();
		void ConsumeString();
		string ConsumeIdent();
		void ConsumeHash();
		bool IsIdentSequence();
		tuple<string, string> ConsumeNumber();
		void ConsumeNumericToken();
		void ConsumeIdentLike();
		void ConsumeUrl();
		void ConsumeBadUrl();
		void SkipSpace();
};