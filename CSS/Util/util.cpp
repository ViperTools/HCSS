#include "util.hpp"

bool wstrcompi(std::wstring str1, std::wstring str2) {
	if (str1.length() == str2.length()) {
		return std::equal(str2.begin(), str2.end(),
			str1.begin(), [](wchar_t a, wchar_t b) -> bool { return std::tolower(a) == std::tolower(b); });
	}
	return false;
}