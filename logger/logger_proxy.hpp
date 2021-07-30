#pragma once

#include <string>
#include <atomic>
#include <mutex>
#include <fstream>
#include "utils.hpp"
#include <stdexcept>

#include <iostream>

namespace proxy::logger
{
	enum class log_level;

	inline std::string log_level_to_string(log_level level);

	template <log_level Level, std::size_t MaxFileSize>
	class logger_proxy
	{
	public:
		using size_type = std::size_t;

		constexpr static log_level this_log_level = Level;
		constexpr static size_type max_file_size  = MaxFileSize;

		inline static std::string log_suffix = ".txt";

		logger_proxy()
			: current_file_size_(0) {}

		explicit logger_proxy(const std::string_view output_dir)
			: current_file_size_(0)
		{
			set_log_directory(output_dir);
		}

		~logger_proxy()
		{
			if (file_handle_.is_open())
			{
				file_handle_.close();
			}
		}

		logger_proxy(const logger_proxy&)                = delete;
		logger_proxy& operator=(const logger_proxy&)     = delete;
		logger_proxy(logger_proxy&&) noexcept            = default;
		logger_proxy& operator=(logger_proxy&&) noexcept = default;

		void set_log_directory(const std::string_view output_dir)
		{
			directory_path_ = output_dir;
			directory_path_.append(log_level_to_string(this_log_level));
		}

		void write(const std::string_view context)
		{
			std::call_once(
							flag_,
							[&]() -> void
							{
								if (!create_dir(directory_path_))
								{
									throw std::runtime_error(
															"cannot create a new directory: " +
															directory_path_.string());
								}
							});

			if (!is_file_valid())
			{
				std::scoped_lock lock(mutex_);
				if (!is_file_valid())
				{
					create_file();
				}
			}

			if (current_file_size_ >= max_file_size)
			{
				std::unique_lock lock(mutex_, std::defer_lock);
				if (lock.try_lock())
				{
					close_file();
					create_file();
				}
			}

			write_file(context);
		}

	private:
		void create_file()
		{
			const auto time     = get_current_time_ms().append(log_suffix);
			auto       new_path = directory_path_;
			const auto filename = new_path.append(time).generic_string();

			file_handle_.open(filename, std::ios::out);

			if (file_handle_.is_open())
			{
				current_file_size_ = 0;
			}
			else
			{
				throw std::runtime_error("cannot create a new file: " + filename);
			}
		}

		void write_file(const std::string_view context)
		{
			file_handle_ << context << '\n';
			std::cout << context << '\n';
			current_file_size_ += context.length();
		}

		void close_file()
		{
			file_handle_.close();
		}

		bool is_file_valid() const
		{
			return file_handle_.is_open();
		}

		std::filesystem::path directory_path_;

		std::atomic<size_type> current_file_size_;

		std::mutex mutex_;

		std::once_flag flag_;
		std::ofstream  file_handle_;
	};
}
