#pragma once

#include <unordered_map>
#include <string_view>
#include <string>
#include "string_parse.hpp"

namespace proxy
{
	class config
	{
	public:
		using config_key_type = std::string;
		using config_value_type = std::string;

		inline static std::string comment_line_start_with = "#";
		inline static std::string kv_separator            = "=";

		void load_config(std::string_view config_path);

		template <typename T>
		bool read_config(const config_key_type& config_key, T& value)
		{
			if (auto it = configs_.find(config_key); it == configs_.end())
			{
				return false;
			}
			else
			{
				utils::parse(it->second, value);
				return true;
			}
		}

	private:
		std::unordered_map<config_key_type, config_value_type> configs_;
	};
}
