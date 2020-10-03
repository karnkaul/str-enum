// KT header-only library
// Requirements: C++17

#pragma once
#include <cstdint>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

///
/// \brief Declare a stringed enum (macro)
///
#define STR_ENUM(name, ...)                                                                                                                \
	enum class name { __VA_ARGS__ };                                                                                                       \
	[[maybe_unused]] inline ::kt::detail::lookup_wrapper<name> g_##name(#__VA_ARGS__);

namespace kt {
///
/// \brief Obtain string representation of Enum e
///
template <typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
std::string_view to_string(Enum e) noexcept;
///
/// \brief Obtain Enum representation of string str
///
template <typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
Enum to_enum(std::string_view str, Enum fallback) noexcept;

// impl
namespace detail {
template <typename Enum>
struct lookup_data {
	std::vector<std::string> to_string;
	std::unordered_map<std::string_view, Enum> to_enum;
};

template <typename Enum>
lookup_data<Enum>& lookup() {
	// Pseudo singleton; one such function instantiated for every Enum,
	// thus one static lookup_data for it as well.
	static lookup_data<Enum> lookup;
	return lookup;
}

inline std::string trim(std::string_view in) {
	std::size_t start = 0;
	std::size_t size = in.size();
	// Keep incrementing start and decrementing size
	while (start < in.size() && std::isspace(in.at(start++))) {
		--size;
	}
	std::size_t end = in.empty() ? 0 : in.size() - 1;
	// Keep decrementing size and end
	while (end > 0 && std::isspace(in.at(end--))) {
		--size;
	}
	return std::string(in.substr(start > 0 ? start - 1 : 0, size));
}

template <typename Enum>
void init_lookup(std::string_view values) {
	auto& l = lookup<Enum>();
	std::stringstream str;
	str << values;
	// First fill up the vector, let it resize etc
	for (std::string value; std::getline(str, value, ',');) {
		l.to_string.push_back(trim(value));
		value.clear();
	}
	// Then add views to its strings in the map
	std::size_t idx = 0;
	for (auto const& s : l.to_string) {
		// s is now stable and can be safely viewed
		l.to_enum.emplace(s, (Enum)idx);
	}
}

template <typename Enum>
struct lookup_wrapper {
	lookup_wrapper(std::string_view str) {
		init_lookup<Enum>(str);
	}
};
} // namespace detail

template <typename Enum, typename>
std::string_view to_string(Enum e) noexcept {
	return detail::lookup<Enum>().to_string.at((std::size_t)e);
}

template <typename Enum, typename>
Enum to_enum(std::string_view str, Enum fallback) noexcept {
	auto const& l = detail::lookup<Enum>().to_enum;
	if (auto search = l.find(str); search != l.end()) {
		return search->second;
	}
	return fallback;
}
} // namespace kt
