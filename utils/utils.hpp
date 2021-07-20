#pragma once

#include <string>
#include <iterator>

namespace proxy::utils
{
	template <typename ContainerType>
	void split(
		std::back_insert_iterator<ContainerType> it,
		const std::string_view                   str,
		const std::string_view                   delimiter)
	{
		using string_type = std::string_view;
		using size_type = string_type::size_type;

		using value_type = typename std::back_insert_iterator<ContainerType>::container_type::value_type;

		size_type current = 0;
		while (true)
		{
			const auto next = str.find(delimiter, current);
			it              = static_cast<value_type>(str.substr(current, next - current));

			if (next == string_type::npos)
			{
				return;
			}
			current = next + delimiter.length();
		}
	}

	inline std::string bin_to_hex(std::string_view bin, std::string_view separator = "-")
	{
		constexpr auto int_to_char = [](const int v) constexpr -> char
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
}
