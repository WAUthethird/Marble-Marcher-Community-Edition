// Compatibility shim: libc++ >= 19 removed the generic std::char_traits<T>.
// SFML 2.x uses std::basic_string<sf::Uint8> / std::basic_string<sf::Uint32>,
// so we provide the two specializations it needs.
#pragma once
#ifdef __cplusplus
#include <cstddef>
#include <cstring>
#include <cwchar>
#include <ios>

namespace std {
template <class T, class IntT>
struct mmce_char_traits_base
{
	typedef T char_type;
	typedef IntT int_type;
	typedef std::streamoff off_type;
	typedef std::streampos pos_type;
	typedef std::mbstate_t state_type;

	static constexpr void assign(char_type& r, const char_type& a) noexcept { r = a; }
	static constexpr bool eq(char_type a, char_type b) noexcept { return a == b; }
	static constexpr bool lt(char_type a, char_type b) noexcept { return a < b; }
	static constexpr int compare(const char_type* a, const char_type* b, size_t n) noexcept
	{
		for (size_t i = 0; i < n; ++i)
		{
			if (a[i] < b[i]) return -1;
			if (b[i] < a[i]) return 1;
		}
		return 0;
	}
	static constexpr size_t length(const char_type* s) noexcept
	{
		size_t n = 0;
		while (s[n] != char_type()) ++n;
		return n;
	}
	static constexpr const char_type* find(const char_type* s, size_t n, const char_type& a) noexcept
	{
		for (size_t i = 0; i < n; ++i)
			if (s[i] == a) return s + i;
		return nullptr;
	}
	static char_type* move(char_type* d, const char_type* s, size_t n) noexcept
	{
		if (n) std::memmove(d, s, n * sizeof(char_type));
		return d;
	}
	static char_type* copy(char_type* d, const char_type* s, size_t n) noexcept
	{
		if (n) std::memcpy(d, s, n * sizeof(char_type));
		return d;
	}
	static char_type* assign(char_type* d, size_t n, char_type a) noexcept
	{
		for (size_t i = 0; i < n; ++i) d[i] = a;
		return d;
	}
	static constexpr int_type not_eof(int_type c) noexcept { return eq_int_type(c, eof()) ? ~eof() : c; }
	static constexpr char_type to_char_type(int_type c) noexcept { return char_type(c); }
	static constexpr int_type to_int_type(char_type c) noexcept { return int_type(c); }
	static constexpr bool eq_int_type(int_type a, int_type b) noexcept { return a == b; }
	static constexpr int_type eof() noexcept { return int_type(-1); }
};

template <> struct char_traits<unsigned char> : mmce_char_traits_base<unsigned char, unsigned int> {};
template <> struct char_traits<unsigned int> : mmce_char_traits_base<unsigned int, unsigned long long> {};
template <> struct char_traits<unsigned short> : mmce_char_traits_base<unsigned short, unsigned int> {};
}
#endif
