#include "Lexer.hpp"
#include <cstdarg>
#include <iostream>
using std::get;

// Helper Functions

bool strcompi(string str1, string str2) {
	if (str1.length() == str2.length()) {
		return std::equal(str2.begin(), str2.end(),
			str1.begin(), [](char a, char b) -> bool { return tolower(a) == tolower(b); });
	}
	return false;
}

bool isHex(char c) {
	return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool nonAscii(char c) {
	return c >= 128;
}

bool identStartCodePoint(char c) {
	return isalpha(c) || nonAscii(c) || c == '_';
}

bool identCodePoint(char c) {
	return identStartCodePoint(c) || isdigit(c) || c == '-';
}

bool isSpace(char c) {
	return c == '\t' || c == '\n' || c == ' ';
}

// Main Lexer

vector<Token> Lexer::Lex() {
	while (true) {
		current = reader.get();
		switch (current) {
			case '(': AddToken(LEFT_PAREN); break;
			case ')': AddToken(RIGHT_PAREN); break;
			case '[': AddToken(RIGHT_BRACKET); break;
			case ']': AddToken(LEFT_BRACKET); break;
			case '{': AddToken(LEFT_BRACE); break;
			case '}': AddToken(RIGHT_BRACE); break;
			case ',': AddToken(COMMA); break;
			case '\n': {
				line++;
			}
			case '\t': case ' ': ConsumeWhitespace(); break;
			case '\'': case '"': ConsumeString(); break;
			case '#': {
				if (identCodePoint(reader.peek())) {
					ConsumeHash();
				}
				else if (reader.peek() == '\\') {
					char next = reader.ignore().peek();
					reader.unget();
					if (next != '\n') {
						ConsumeHash();
					}
				}
				else {
					AddToken(DELIM);
				}
				break;
			}
			case '+': case '.': {
				if (isdigit(reader.peek())) {
					reader.unget();
					ConsumeNumericToken();
				}
				else {
					AddToken(DELIM);
				}
				break;
			}
			case '-': {
				if (isdigit(reader.peek())) {
					reader.unget();
					ConsumeNumericToken();
				}
				else if (reader.peek() == '-') {
					if (reader.ignore().peek() == '>') {
						AddToken(CDC);
					}
					else {
						reader.unget();
					}
					if (IsIdentSequence()) {
						reader.unget();
						ConsumeIdentLike();
					}
				}
				else if (IsIdentSequence()) {
					reader.unget();
					ConsumeIdentLike();
				}
				else {
					AddToken(DELIM);
				}
				break;
			}
			case '<': {
				if (reader.peek() == '!') {
					if (reader.ignore().get() == '-' && reader.peek() == '-') {
						AddToken(CDO);
					}
					else {
						reader.unget();
						reader.unget();
						AddToken(DELIM);
					}
				}
				else {
					AddToken(DELIM);
				}
				break;
			}
			case '@': {
				if (IsIdentSequence()) {
					tokens.push_back(Token(AT_KEYWORD, ConsumeIdent()));
				}
				else {
					AddToken(DELIM);
				}
				break;
			}
			case '\\': {
				if (reader.peek() != '\n') {
					reader.unget();
					ConsumeIdentLike();
				}
				else {
					AddToken(DELIM);
				}
				break;
			}
			case '/': {
				if (reader.peek() == '*') {
					while (reader.peek() != EOF) {
						if (reader.get() == '*' && reader.get() == '/') {
							break;
						}
					}
				}
				break;
			}
			default: {
				if (isdigit(current)) {
					reader.unget();
					ConsumeNumericToken();
				}
				else if (identStartCodePoint(current)) {
					reader.unget();
					ConsumeIdentLike();
				}
				else if (current == EOF) {
					tokens.push_back(Token(T_EOF));
					return tokens;
				}
				else {
					AddToken(DELIM);
				}
				break;
			}
		}
	}
	return tokens;
}

void Lexer::AddToken(TokenType type) {
	tokens.push_back(Token(type, string(1, current), reader.tellg(), line));
}

void Lexer::ConsumeWhitespace() {
	Token t(WHITESPACE, string(1, current), reader.tellg(), line);
	while (isSpace(reader.peek())) {
		t.lexeme += reader.get();
	}
	tokens.push_back(t);
}

char Lexer::ConsumeEscapedCodePoint() {
	char c = reader.get();
	if (c != '\n') {
		if (isHex(c)) {
			string hex = string(1, c);
			while (hex.size() < 6 && isHex(reader.peek())) {
				hex += reader.get();
			}
			int n = std::stoi(hex, nullptr, 16);
			if (n == 0 || n > 1114111 || (n >= 55296 && n <= 57343)) {
				return '\0';
			}
			return n;
		}
		else if (c == EOF) {
			return '\0';
		}
		else {
			return c;
		}
	}
	else {
		reader.ignore();
	}
	return '\0';
}

void Lexer::ConsumeString() {
	Token t(STRING, "", reader.tellg(), line);
	while (reader.peek() != EOF) {
		char c = reader.get();
		switch (c) {
			case '"':
			case EOF: tokens.push_back(t); return;
			case '\n': {
				reader.unget();
				tokens.push_back(Token(BAD_STRING, t.lexeme));
				return;
			}
			case '\\': {
				t.lexeme += ConsumeEscapedCodePoint();
				break;
			}
			default: t.lexeme += c;
		}
	}
}

string Lexer::ConsumeIdent() {
	string result;
	while (reader.peek() != EOF) {
		char c = reader.get();
		if (identCodePoint(c)) {
			result += c;
		}
		else if (c == '\\' && reader.peek() != '\n') {
			result += ConsumeEscapedCodePoint();
		}
		else {
			reader.unget();
			break;
		}
	}
	return result;
}

bool Lexer::IsIdentSequence() {
	char next = reader.peek();
	if (next == '-') {
		next = reader.ignore().peek();
		reader.unget();
		if (next == '-' || identStartCodePoint(next)) {
			return true;
		}
		else if (next == '\\') {
			next = reader.ignore().peek();
			reader.unget();
			if (reader.peek() != '\n') {
				return true;
			}
		}
	}
	if (identStartCodePoint(next)) {
		return true;
	}
	else if (next == '\\' && reader.peek() != '\n') {
		return true;
	}
	return false;
}

void Lexer::ConsumeHash() {
	Token t(HASH, "", reader.tellg(), line);
	if (IsIdentSequence()) {
		t.flags["type"] = "id";
	}
	t.lexeme = ConsumeIdent();
	tokens.push_back(t);
}

tuple<string, string> Lexer::ConsumeNumber() {
	string type = "integer";
	string repr;
	char next = reader.peek();
	if (next == '+' || next == '-') {
		repr += reader.get();
		next = reader.peek();
	}
	while (isdigit(next)) {
		repr += reader.get();
		next = reader.peek();
	}
	if (next == '.') {
		if (isdigit(reader.ignore().peek())) {
			type = "number";
			repr += next;
			while (isdigit(reader.peek())) {
				repr += reader.get();
			}
			next = reader.peek();
		}
		else {
			reader.unget();
		}
	}
	if (next == 'e' || next == 'E') {
		char c = reader.ignore().peek();
		string suffix(1, next);
		if (c == '+' || c == '-') {
			reader.ignore();
			suffix += c;
		}
		if (isdigit(reader.peek())) {
			repr += suffix;
			type = "number";
			while (isdigit(reader.peek())) {
				repr += reader.get();
			}
		}
		else {
			reader.unget();
		}
	}
	return std::make_tuple(repr, type);
}

void Lexer::ConsumeNumericToken() {
	tuple<string, string> number = ConsumeNumber();
	TokenType type;
	if (IsIdentSequence()) {
		type = DIMENSION;
	}
	else if (reader.peek() == '%') {
		type = PERCENTAGE;
	}
	else {
		type = NUMBER;
	}
	Token t(type, get<0>(number), reader.tellg(), line);
	t.flags["type"] = get<1>(number);
	if (type == DIMENSION) {
		t.flags["unit"] = ConsumeIdent();
	}
	tokens.push_back(t);
}

void Lexer::ConsumeIdentLike() {
	string s = ConsumeIdent();
	if (reader.peek() == '(') {
		reader.ignore();
		if (strcompi(s, "url")) {
			while (isSpace(reader.peek()) && isSpace(reader.ignore().peek()));
			if (reader.peek() == '"' || reader.peek() == '\'') {
				tokens.push_back(Token(FUNCTION, s, reader.tellg(), line));
			}
			else {
				ConsumeUrl();
			}
		}
		else {
			tokens.push_back(Token(FUNCTION, s, reader.tellg(), line));
		}
	}
	else {
		tokens.push_back(Token(IDENT, s, reader.tellg(), line));
	}
}

void Lexer::SkipSpace() {
	while (isSpace(reader.peek())) reader.ignore();
}

void Lexer::ConsumeUrl() {
	Token t(URL, "", reader.tellg(), line);
	SkipSpace();
	while (reader.peek() != EOF) {
		char c = reader.get();
		switch (c) {
			case EOF:
			case ')': tokens.push_back(t); return;
			case ' ': case '\n': case '\t': SkipSpace(); break;
			case '"':
			case '\'':
			case '(': {
				ConsumeBadUrl();
				AddToken(BAD_URL);
				return;
			}
			case '\\': {
				if (reader.peek() != '\n') {
					t.lexeme += ConsumeEscapedCodePoint();
				}
				else {
					ConsumeBadUrl();
					AddToken(BAD_URL);
					return;
				}
				break;
			}
			default: t.lexeme += c; break;
		}
	}
}

void Lexer::ConsumeBadUrl() {
	while (reader.peek() != EOF) {
		char c = reader.get();
		switch (c) {
			case ')': return;
			case '\\': {
				if (reader.peek() != '\n') {
					ConsumeEscapedCodePoint();
				}
				break;
			}
		}
	}
}