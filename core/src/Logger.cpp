#include <iostream>
#include <mutex>
#include <iterator>
#include "../../include/core/logging/Logger.h"

namespace ducklib
{
std::mutex Logger::stdout_mutex;

auto Logger::add_prefix(const std::span<char>& buffer, LogLevel level) -> std::span<char>::size_type
{
    static const std::array<std::string, 4> prefixes = {"ERROR", "WARNING", "INFO", "DEBUG"};
    static const std::string delimiter = ": ";

    auto level_prefix = prefixes[static_cast<int>(level)];
    auto iterator = std::ranges::copy(level_prefix, buffer.begin()).out;
    iterator = std::ranges::copy(delimiter, iterator).out;

    return std::distance(buffer.begin(), iterator);
}

void Logger::stdout_sink(std::span<char> text)
{
    std::lock_guard lock(stdout_mutex);
    std::cout << text.data() << std::endl;
}
}
