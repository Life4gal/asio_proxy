#pragma once

#include <exception>
#include <charconv>
#include <vector>
#include <string>

namespace proxy::utils
{
	namespace detail
	{
		template <typename T>
		struct is_container
		{
			static constexpr bool value = false;
		};

		template <typename T>
		struct is_container<std::vector<T>>
		{
			static constexpr bool value = true;
		};

		template <typename T>
		constexpr static bool is_container_v = is_container<T>::value;

		constexpr static char container_delimiter = ',';

		constexpr bool get_text_boolean_result(std::string_view text);

		constexpr char get_text_char_result(std::string_view text);

		template <typename T>
		constexpr T get_text_integer_or_floating_point_result(std::string_view text);

		template <typename T>
		constexpr void get_text_container_result(std::string_view text, std::vector<T>& result);
	}

	class parser_exception final : std::exception
	{
	public:
		explicit parser_exception(std::string message)
			: message(std::move(message)) {}

		[[nodiscard]] const char* what() const noexcept override
		{
			return message.c_str();
		}

	private:
		std::string message;
	};

	template <typename T>
	void parse(const std::string_view text, T& out)
	{
		if constexpr (std::is_same_v<T, bool>)
		{
			out = detail::get_text_boolean_result(text);
		}
		else if constexpr (std::is_same_v<T, char>)
		{
			out = detail::get_text_char_result(text);
		}
		else if constexpr (std::is_integral_v<T> or std::is_floating_point_v<T>)
		{
			out = detail::get_text_integer_or_floating_point_result<T>(text);
		}
		else if constexpr (detail::is_container_v<T>)
		{
			detail::get_text_container_result(text, out);
		}
		else
		{
			out = text;
		}
	}

	#pragma push_macro("TRUE")
	#pragma push_macro("FALSE")
	#undef TRUE
	#undef FALSE

	namespace detail
	{
		constexpr bool get_text_boolean_result(const std::string_view text)
		{
			constexpr std::string_view TRUE  = "true";
			constexpr std::string_view FALSE = "false";

			if (text.length() == 1)
			{
				if (text[0] == '1')
				{
					return true;
				}
				else if (text[0] == '0')
				{
					return false;
				}
			}
			else if ((text.front() == 't' or text.front() == 'T') and text.length() == TRUE.length())
			{
				// if text equal to TRUE, then return true
				return std::equal(
								text.cbegin(),
								text.cend(),
								std::cbegin(TRUE),
								[](const char a, const char b)
								{
									return std::tolower(a) == std::tolower(b);
								});
			}
			else if ((text.front() == 'f' or text.front() == 'F') and text.length() == FALSE.length())
			{
				// if text equal to FALSE, then return false
				return !std::equal(
									text.cbegin(),
									text.cend(),
									std::cbegin(FALSE),
									[](char a, char b)
									{
										return std::tolower(a) == std::tolower(b);
									});
			}

			throw parser_exception{std::string{"Cannot parse `"}.append(text) + "` to boolean!"};
		}

		constexpr char get_text_char_result(const std::string_view text)
		{
			if (text.length() == 1)
			{
				return text[0];
			}

			throw parser_exception{std::string{"Cannot parse `"}.append(text) + "` to char!"};
		}

		template <typename T>
		constexpr T get_text_integer_or_floating_point_result(const std::string_view text)
		{
			if (not text.empty())
			{
				const auto* begin    = text.data();
				bool        negative = false;
				bool        hex      = false;

				if constexpr (std::is_signed_v<T>)
				{
					if (text[0] == '-')
					{
						begin += 1;
						negative = true;
					}
				}

				if (text.length() >= 2 and begin[0] == '0' and (begin[1] == 'x' or begin[1] == 'X'))
				{
					begin += 2;
					hex = true;
				}

				T ret{};
				// std::from_chars cannot parse hex number like -0x123, it just parse `-` and `0` and return 0
				// we must do it by self
				if constexpr (std::is_floating_point_v<T>)
				{
					std::from_chars(begin,
									text.data() + text.length(),
									ret,
									hex
										? std::chars_format::hex
										: std::chars_format::general);
				}
				else
				{
					std::from_chars(begin,
									text.data() + text.length(),
									ret,
									hex
										? 16
										: 10);
				}

				return negative
							? -ret
							: ret;
			}

			throw parser_exception{std::string{"Cannot parse `"}.append(text) + "` to integer or floating point!"};
		}

		template <typename T>
		constexpr void get_text_container_result(const std::string_view text, std::vector<T>& result)
		{
			std::string_view::size_type it = 0;
			for (;;)
			{
				const auto next = text.find_first_of(container_delimiter, it);

				if (next == std::string_view::npos)
				{
					break;
				}

				result.emplace_back(text.substr(it, next));
				it = next + sizeof(container_delimiter);
			}

			if (it != text.size())
			{
				result.emplace_back(text.substr(it));
			}
		}
	}

	#pragma pop_macro("TRUE")
	#pragma pop_macro("FALSE")
}
