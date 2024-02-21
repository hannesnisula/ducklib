#pragma once
#include <cstdint>
#include <format>
#include <mutex>
#include <regex>

namespace ducklib
{
enum class LogLevel
{
    Error,
    Warning,
    Info,
    Debug,
};

class Logger
{
public:
    void set_log_level(LogLevel new_log_level);

    template <typename... Args>
    void log_error(const std::string& text, const Args&... args);
    template <typename... Args>
    void log_warning(const std::string& text, const Args&... args);
    template <typename... Args>
    void log_info(const std::string& text, const Args&... args);
    template <typename... Args>
    void log_debug(const std::string& text, const Args&... args);

    template <typename... Args>
    void log(LogLevel level, const std::string& text, const Args&... args);

private:
    static constexpr uint16_t MessageFormatBufferSize = 512;

    static std::mutex stdout_mutex;

    /**
     * \return Prefix length in chars
     */
    static auto add_prefix(const std::span<char>& buffer, LogLevel level) -> std::span<char>::size_type;
    static void stdout_sink(std::span<char> text); // TODO: Move to somewhere external

    LogLevel log_level = LogLevel::Warning;
};

inline void Logger::set_log_level(LogLevel new_log_level)
{
    log_level = new_log_level;
}

template <typename... Args>
void Logger::log_error(const std::string& text, const Args&... args)
{
    Log(LogLevel::Error, text, args...);
}

template <typename... Args>
void Logger::log_warning(const std::string& text, const Args&... args)
{
    Log(LogLevel::Warning, text, args...);
}

template <typename... Args>
void Logger::log_info(const std::string& text, const Args&... args)
{
    Log(LogLevel::Info, text, args...);
}

template <typename... Args>
void Logger::log_debug(const std::string& text, const Args&... args)
{
    Log(LogLevel::Debug, text, args...);
}

template <typename... Args>
void Logger::log(LogLevel level, const std::string& text, const Args&... args)
{
    if (level > log_level)
        return;

    std::array<char, MessageFormatBufferSize> format_buffer{};
    auto message_buffer_iterator = add_prefix(format_buffer, level);
    std::format_to(message_buffer_iterator, text, args...);

    // TODO: Support sinks/outputs
    stdout_sink(format_buffer);
}
}
