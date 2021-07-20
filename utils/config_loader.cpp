#include "config_loader.hpp"
#include <fstream>

namespace
{
	std::vector<std::string> read_file(
		const std::string_view file_path,
		const std::string_view ignore_line_start_with = "#"
		)
	{
		std::ifstream file(file_path.data());

		std::vector<std::string> ret;
		std::string              line;
		while (std::getline(file, line))
		{
			if (std::equal(ignore_line_start_with.cbegin(), ignore_line_start_with.cend(), line.cbegin()))
			{
				continue;
			}

			ret.push_back(line);
		}

		return ret;
	}
}

namespace proxy
{
	void config::load_config(const std::string_view config_path)
	{
		auto config = read_file(config_path, comment_line_start_with);

		for (auto& line : config)
		{
			const auto separator = line.find(kv_separator);
			if (separator == decltype(kv_separator)::npos)
			{
				continue;
			}

			configs_.emplace(
							line.substr(0, separator),
							line.substr(separator + kv_separator.length())
							);
		}
	}
}
