#pragma once

#include <string>
#include <atomic>
#include <mutex>
#include <string_view>
#include <fstream>
#include "utils.hpp"
#include <stdexcept>

namespace proxy::logger
{
	enum class log_level;

	inline std::string log_level_to_string(const log_level level);

	template <log_level Level, std::size_t MaxFileSize>
	class logger_proxy
	{
	public:
		using size_type = std::size_t;

		constexpr static log_level this_log_level = Level;
		constexpr static size_type max_file_size  = MaxFileSize;

		inline static std::string log_suffix = ".txt";

		explicit logger_proxy(const std::string_view output_dir)
			: directory_name_(std::string{output_dir} + '.' + get_current_exe_name() + '.' +
							log_level_to_string(this_log_level)),
			current_file_size_(0) {}

		void write(const std::string_view context)
		{
			std::call_once(
							flag_,
							[&]() -> void
							{
								if (create_dir(directory_name_))
								{
									throw std::runtime_error("cannot create a new directory: " + directory_name_);
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
			const auto time      = get_current_time_ms();
			const auto file_name = std::filesystem::path{directory_name_}.append(time);

			file_handle_.open(file_name, std::ios::out);

			if (file_handle_.is_open())
			{
				current_file_size_ = 0;
				create_link(time);
			}
			else
			{
				throw std::runtime_error("cannot create a new file: " + file_name.string());
			}
		}

		void create_link(std::string_view time)
		{
			// todo
		}

		void write_file(const std::string_view context)
		{
			file_handle_ << context;
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

		std::string directory_name_;

		std::atomic<size_type> current_file_size_;

		std::mutex mutex_;

		std::once_flag flag_;
		std::ofstream  file_handle_;
	};
}
