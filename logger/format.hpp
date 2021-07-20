#pragma once

#include <string>
#include <tuple>
#include <stdexcept>

namespace proxy::logger
{
	namespace detail
	{
		template <std::size_t I, typename Tuple>
		std::enable_if_t<(I == std::tuple_size<Tuple>::value), std::string>
		get_arg_by_index(Tuple&, std::size_t)
		{
			throw std::invalid_argument("Arg index out of range");
		}

		template <std::size_t I = 0, typename Tuple>
		std::enable_if_t<(I < std::tuple_size<Tuple>::value), std::string>
		get_arg_by_index(Tuple& tuple, std::size_t index)
		{
			if (I == index)
			{
				return std::to_string(std::get<I>(tuple));
			}
			return get_arg_by_index<I + 1>(tuple, index);
		}
	}

	template <typename... Args>
	std::string format(const std::string_view format, Args&&...args)
	{
		using size_type = std::string::size_type;

		if constexpr (sizeof...(args) == 0)
		{
			return std::string{format};
		}

		auto tuple = std::make_tuple(args...);

		std::string buffer;
		auto        current = format.data();
		auto        last    = current;

		size_type pos = 0;

		while (true)
		{
			if (*current == '{' && *(current + 1) == '}')
			{
				if (const auto len = current - last; len != 0)
				{
					buffer.append(last, static_cast<size_type>(len));
				}

				buffer.append(detail::get_arg_by_index<0>(tuple, pos++));

				last = current + 2;
				++current;
			}
			else if (*current == '\0')
			{
				if (const auto len = current - last; len != 0)
				{
					buffer.append(last, static_cast<size_type>(len));
				}
				break;
			}

			++current;
		}

		return buffer;
	}
}
