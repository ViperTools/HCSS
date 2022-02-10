#pragma once
#include <string>
#include <variant>

template <typename T, typename... Args> struct concatenator;

template <typename... Args0, typename... Args1>
struct concatenator<std::variant<Args0...>, Args1...> {
    using type = std::variant<Args0..., Args1...>;
};

template <typename V, typename... Args1>
using variant_append = typename concatenator<V, Args1...>::type;

bool wstrcompi(std::wstring str1, std::wstring str2);