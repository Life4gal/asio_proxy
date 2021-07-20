#pragma once

#include <string>
#include <boost/dll.hpp>
#include <chrono>
#include <filesystem>

namespace proxy::logger
{
	inline std::string get_current_exe_name()
	{
		return boost::dll::program_location().string();
	}

	// 1970-1-1 00:00:00
	inline std::string get_current_time_ms()
	{
		using namespace std::chrono;

		auto time = system_clock::to_time_t(system_clock::now());

		std::string str(30, '\0');
		std::strftime(str.data(), str.length(), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
		return str;
	}

	inline bool file_exist(const std::string_view file_path)
	{
		return std::filesystem::exists(file_path);
	}

	inline bool create_dir(const std::string_view file_path)
	{
		if (file_exist(file_path))
		{
			return true;
		}

		using namespace std::filesystem;

		if (const path path{file_path}; !create_directories(path))
		{
			return false;
		}
		else
		{
			permissions(path, perms::others_all, perm_options::remove);
			permissions(path, perms::group_all, perm_options::replace);
			permissions(path, perms::owner_all, perm_options::replace);
		}
	}
}
