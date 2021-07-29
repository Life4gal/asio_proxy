#pragma once

#include <string>
#include "logger_proxy.hpp"
#include <source_location>
#include <string_view>

namespace proxy
{
	namespace logger
	{
		enum class log_level
		{
			info = 0,
			warning = 1,
			error = 2
		};

		inline std::string log_level_to_string(const log_level level)
		{
			switch (level)
			{
				case log_level::info:
					return "info";
				case log_level::warning:
					return "warning";
				case log_level::error:
					return "error";
				default:
					return "unknown";
			}
		}
	}

	class logger_manager
	{
	public:
		static logger_manager& instance()
		{
			static logger_manager instance_;
			return instance_;
		}

		using size_type = std::size_t;

		constexpr static size_type info_max_file_size    = 1024 * 1024 * 10; // 10 M
		constexpr static size_type warning_max_file_size = 1024 * 1024 * 5;  // 5 M
		constexpr static size_type error_max_file_size   = 1024 * 1024 * 1;  // 1 M

		template <const logger::log_level Level>
		void set_log_directory(const std::string_view output_dir)
		{
			if constexpr (Level == logger::log_level::info)
			{
				info_.set_log_directory(output_dir);
			}
			else if constexpr (Level == logger::log_level::warning)
			{
				warning_.set_log_directory(output_dir);
			}
			else if constexpr (Level == logger::log_level::error)
			{
				error_.set_log_directory(output_dir);
			}
			else
			{
				throw std::invalid_argument{"invalid log level"};
			}
		}

		void set_log_level(const logger::log_level level)
		{
			level_ = level;
		}

		template <const logger::log_level Level>
		void write(
			std::string_view            context,
			const std::source_location& location)
		{
			if (check_level(Level))
			{
				if constexpr (auto text = std::format("[FILE: {} -> LINE: {}] -> {}",
													location.file_name(),
													location.line(),
													context); Level == logger::log_level::info)
				{
					info_.write(text);
				}
				else if constexpr (Level == logger::log_level::warning)
				{
					warning_.write(text);
				}
				else if constexpr (Level == logger::log_level::error)
				{
					error_.write(text);
				}
				else
				{
					throw std::invalid_argument{"invalid log level"};
				}
			}
		}

	private:
		[[nodiscard]] constexpr bool check_level(logger::log_level level) const
		{
			using underlying_type = std::underlying_type_t<logger::log_level>;
			return static_cast<underlying_type>(level) >= static_cast<underlying_type>(level_);
		}

		logger::log_level level_{logger::log_level::info};

		logger::logger_proxy<logger::log_level::info, info_max_file_size>       info_{"log"};
		logger::logger_proxy<logger::log_level::warning, warning_max_file_size> warning_{"log"};
		logger::logger_proxy<logger::log_level::error, error_max_file_size>     error_{"log"};
	};

	template <const logger::log_level Level>
	void log(const std::string_view context, const std::source_location& location = std::source_location::current())
	{
		logger_manager::instance().write<Level>(context, location);
	}

	inline void log_info(const std::string_view     context,
						const std::source_location& location = std::source_location::current())
	{
		log<logger::log_level::info>(context, location);
	}

	inline void log_warning(const std::string_view      context,
							const std::source_location& location = std::source_location::current())
	{
		log<logger::log_level::warning>(context, location);
	}

	inline void log_error(const std::string_view    context,
						const std::source_location& location = std::source_location::current())
	{
		log<logger::log_level::error>(context, location);
	}
}
