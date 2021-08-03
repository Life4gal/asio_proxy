#pragma once

#include <string>

namespace proxy::utils
{
	template <typename Char, typename Trait = std::char_traits<Char>>
	std::string bin_to_hex_helper(const std::basic_string_view<Char, Trait> bin, const std::string_view separator)
	{
		constexpr auto int_to_char = [](const auto v) constexpr -> char
		{
			if (v < 10)
			{
				return static_cast<char>(v + '0');
			}
			return static_cast<char>(v + 'A' - 10);
		};

		std::string result;
		for (auto c : bin)
		{
			if (!result.empty())
			{
				result.append(separator);
			}
			result.push_back(int_to_char(c >> 4));
			result.push_back(int_to_char(c % 16));
		}

		return result;
	}

	inline std::string bin_to_hex(const std::string_view bin, const std::string_view separator = "-")
	{
		return bin_to_hex_helper<char>(bin, separator);
	}

	inline std::string bin_to_hex(const std::basic_string_view<unsigned char> bin,
								const std::string_view                        separator = "-")
	{
		return bin_to_hex_helper<unsigned char>(bin, separator);
	}
}
