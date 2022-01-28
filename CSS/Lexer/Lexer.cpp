#include "../Util/util.hpp"
#include "Lexer.hpp"
#include "TokenType.hpp"
#include <iostream>
using std::get;
using std::to_wstring;

#pragma region Helpers

bool isHex(wchar_t c) {
	return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool nonAscii(wchar_t c) {
	return c >= 128;
}

bool identStartCodePoint(wchar_t c) {
	return isalpha(c) || nonAscii(c) || c == '_';
}

bool identCodePoint(wchar_t c) {
	return identStartCodePoint(c) || isdigit(c) || c == '-';
}

bool isSpace(wchar_t c) {
	return c == '\t' || c == '\n' || c == ' ';
}

#pragma endregion

#pragma region Lexer

vector<Token> Lexer::lex() {
	while (reader.peek() != EOF) {
		current = reader.get();
		switch (current) {
			case '(':
                addToken(LEFT_PAREN); break;
			case ')':
                addToken(RIGHT_PAREN); break;
			case '[':
                addToken(LEFT_BRACKET); break;
			case ']':
                addToken(RIGHT_BRACKET); break;
			case '{':
                addToken(LEFT_BRACE); break;
			case '}':
                addToken(RIGHT_BRACE); break;
			case ',':
                addToken(COMMA); break;
			case ':':
                addToken(COLON); break;
			case ';':
                addToken(SEMICOLON); break;
			case '\n': line++;
            case '\t': case ' ':
                consumeWhitespace(); break;
			case '\'': case '"':
                consumeString(); break;
			case '#': {
				if (identCodePoint(reader.peek())) {
                    consumeHash();
				}
				else if (reader.peek() == '\\') {
                    wchar_t next = reader.ignore().peek();
					reader.unget();
					if (next != '\n') {
                        consumeHash();
					}
				}
				else {
                    addToken(DELIM);
				}
				break;
			}
			case '+': case '.': {
				if (isdigit(reader.peek())) {
					reader.unget();
                    consumeNumericToken();
				}
				else {
                    addToken(DELIM);
				}
				break;
			}
			case '-': {
				if (isdigit(reader.peek())) {
					reader.unget();
                    consumeNumericToken();
				}
				else if (reader.peek() == '-') {
					if (reader.ignore().peek() == '>') {
                        addToken(CDC);
					}
					else {
						reader.unget();
					}
					if (isIdentSequence()) {
						reader.unget();
                        consumeIdentLike();
					}
				}
				else if (isIdentSequence()) {
					reader.unget();
                    consumeIdentLike();
				}
				else {
                    addToken(DELIM);
				}
				break;
			}
			case '<': {
				if (reader.peek() == '!') {
					if (reader.ignore().get() == '-' && reader.peek() == '-') {
                        addToken(CDO);
					}
					else {
						reader.unget();
						reader.unget();
                        addToken(DELIM);
					}
				}
				else {
                    addToken(DELIM);
				}
				break;
			}
			case '@': {
				if (isIdentSequence()) {
					tokens.emplace_back(AT_KEYWORD, consumeIdent());
				}
				else {
                    addToken(DELIM);
				}
				break;
			}
			case '\\': {
				if (reader.peek() != '\n') {
					reader.unget();
                    consumeIdentLike();
				}
				else {
                    addToken(DELIM);
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
                    consumeNumericToken();
				}
				else if (identStartCodePoint(current)) {
					reader.unget();
                    consumeIdentLike();
				}
				else {
                    addToken(DELIM);
				}
				break;
			}
		}
	}
    tokens.emplace_back(T_EOF);
    return tokens;
}

void Lexer::addToken(TokenType type) {
	tokens.emplace_back(type, wstring(1, current), COLUMN, line);
}

void Lexer::consumeWhitespace() {
	Token t(WHITESPACE, wstring(1, current), COLUMN, line);
	while (isSpace(reader.peek())) {
        auto c = (wchar_t) reader.get();
        if (c == '\n') {
            posOffset = (int) reader.tellg();
            line++;
        }
		t.lexeme += c;
	}
	tokens.push_back(t);
}

wchar_t Lexer::consumeEscapedCodePoint() {
    int c = reader.get();
	if (c != '\n') {
		if (isHex(c)) {
			wstring hex = wstring(1, c);
			while (hex.size() < 6 && isHex(reader.peek())) {
				hex += to_wstring(reader.get());
			}
			int n = std::stoi(hex, nullptr, 16);
			if (n == 0 || n > 1114111 || (n >= 55296 && n <= 57343)) {
				return '\0';
			}
			return  n;
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

void Lexer::consumeString() {
	Token t(STRING, {}, COLUMN, line);
	char end = current;
	while (true) {
        int c = reader.get();
		switch (c) {
			case EOF: tokens.push_back(t); return;
            case '\n': {
				reader.unget();
				tokens.emplace_back(BAD_STRING, t.lexeme);
				return;
			}
			case '\\': {
				t.lexeme += consumeEscapedCodePoint();
				break;
			}
			default: {
				if (c != end) {
					t.lexeme += (wchar_t) c;
				}
				else {
					tokens.push_back(t);
					return;
				}
			}
		}
	}
}

wstring Lexer::consumeIdent() {
	wstring result;
	while (true) {
        wchar_t c = reader.get();
		if (identCodePoint(c)) {
			result += c;
		}
		else if (c == '\\' && reader.peek() != '\n') {
			result += consumeEscapedCodePoint();
		}
		else {
			reader.unget();
			break;
		}
	}
	return result;
}

bool Lexer::isIdentSequence() {
    wchar_t next = reader.peek();
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

void Lexer::consumeHash() {
	Token t(HASH, {}, COLUMN, line);
	if (isIdentSequence()) {
		t.flags["type"] = L"id";
	}
	t.lexeme = consumeIdent();
	tokens.push_back(t);
}

tuple<wstring, wstring> Lexer::consumeNumber() {
	wstring type = L"integer";
	wstring repr;
    wchar_t next = reader.peek();
	if (next == '+' || next == '-') {
		repr += to_wstring(reader.get());
		next = reader.peek();
	}
	while (isdigit(next)) {
		repr += to_wstring(reader.get());
		next = reader.peek();
	}
	if (next == '.') {
		if (isdigit(reader.ignore().peek())) {
			type = L"number";
			repr += next;
			while (isdigit(reader.peek())) {
				repr += to_wstring(reader.get());
			}
			next = reader.peek();
		}
		else {
			reader.unget();
		}
	}
	if (next == 'e' || next == 'E') {
        wchar_t c = reader.ignore().peek();
		wstring suffix(1, next);
		if (c == '+' || c == '-') {
			reader.ignore();
			suffix += c;
		}
		if (isdigit(reader.peek())) {
			repr += suffix;
			type = L"number";
			while (isdigit(reader.peek())) {
				repr += to_wstring(reader.get());
			}
		}
		else {
			reader.unget();
		}
	}
	return std::make_tuple(repr, type);
}

void Lexer::consumeNumericToken() {
	tuple<wstring, wstring> number = consumeNumber();
	TokenType type;
	if (isIdentSequence()) {
		type = DIMENSION;
	}
	else if (reader.peek() == '%') {
		reader.ignore();
		type = PERCENTAGE;
	}
	else {
		type = NUMBER;
	}
	Token t(type, get<0>(number), COLUMN, line);
	t.flags["type"] = get<1>(number);
	if (type == DIMENSION) {
		t.flags["unit"] = consumeIdent();
	}
	tokens.push_back(t);
}

void Lexer::consumeIdentLike() {
	wstring s = consumeIdent();
	if (reader.peek() == '(') {
		reader.ignore();
		if (wstrcompi(s, L"url")) {
			skipSpace();
			if (reader.peek() == '"' || reader.peek() == '\'') {
				tokens.emplace_back(FUNCTION, s, COLUMN, line);
			}
			else {
                consumeUrl();
			}
		}
		else {
			tokens.emplace_back(FUNCTION, s, COLUMN, line);
		}
	}
	else {
		tokens.emplace_back(IDENT, s, COLUMN, line);
	}
}

void Lexer::skipSpace() {
	while (isSpace(reader.peek())) {
        if (reader.peek() == '\n') {
            posOffset = (int) reader.tellg();
            line++;
        }
        reader.ignore();
    }
}

void Lexer::consumeUrl() {
	Token t(URL, {}, COLUMN, line);
    skipSpace();
	while (true) {
		int c = reader.get();
		switch (c) {
			case EOF:
			case ')': tokens.push_back(t); return;
            case '\n': {
                posOffset = (int) reader.tellg();
                line++;
            }
			case ' ': case '\t':
                skipSpace(); break;
			case '"':
			case '\'':
			case '(': {
                consumeBadUrl();
                addToken(BAD_URL);
				return;
			}
			case '\\': {
				if (reader.peek() != '\n') {
					t.lexeme += consumeEscapedCodePoint();
				}
				else {
                    consumeBadUrl();
                    addToken(BAD_URL);
					return;
				}
				break;
			}
			default: t.lexeme += to_wstring(c); break;
		}
	}
}

void Lexer::consumeBadUrl() {
	while (true) {
		int c = reader.get();
		switch (c) {
			case ')': return;
			case '\\': {
				if (reader.peek() != '\n') {
                    consumeEscapedCodePoint();
				}
				break;
			}
            case EOF: return;
            default: break;
		}
	}
}

#pragma endregion
