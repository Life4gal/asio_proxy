#include "pch.h"

#include "../logger/logger.hpp"
#include <format>

TEST(TestLogger, LogInfo)
{
	proxy::log_info(std::format("{} {}", "Hello", "World"));
}
