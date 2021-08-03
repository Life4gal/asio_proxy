#pragma once

#include <string>
// #include <boost/dll.hpp>
#include <chrono>
#include <filesystem>

namespace proxy::logger
{
	// inline std::string get_current_exe_name()
	// {
	// 	// todo: a better way
	// 	return boost::dll::program_location().filename().string();
	// }

	// 1970-1-1 00:00:00
	inline std::string get_current_time_ms()
	{
		using namespace std::chrono;

		auto time = system_clock::to_time_t(system_clock::now());
		tm   real_time{};
		// todo: only work on Windows
		localtime_s(&real_time, &time);

		// std::string str(30, '\0');
		// std::strftime(str.data(), str.length(), "%Y-%m-%d %H-%M-%S", &real_time);
		// return str;

		char str[30]{};
		std::strftime(str, std::size(str), "%Y-%m-%d %H-%M-%S", &real_time);
		return str;
	}

	inline bool file_exist(const std::filesystem::path& file_path)
	{
		return exists(file_path);
	}

	inline bool create_dir(const std::filesystem::path& file_path)
	{
		if (file_exist(file_path))
		{
			return true;
		}

		using perms = std::filesystem::perms;
		using perm_options = std::filesystem::perm_options;

		if (!create_directories(file_path))
		{
			return false;
		}
		permissions(file_path, perms::others_all, perm_options::remove);
		permissions(file_path, perms::group_all, perm_options::replace);
		permissions(file_path, perms::owner_all, perm_options::replace);
		return true;
	}

	inline std::filesystem::path get_full_path(const std::filesystem::path& file_path)
	{
		return absolute(file_path);
	}
}
