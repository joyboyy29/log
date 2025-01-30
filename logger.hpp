#pragma once

#include <iostream>
#include <fstream>
#include <source_location>
#include <string_view>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <map>
#include <future>
#include <Windows.h>

class Logger {
public:
    enum class LogLevel {
        Info,      // [+] Success / Information
        Warning,   // [!] Warnings
        Error,     // [-] Errors
        Debug      // [*] Debug Info
    };

    struct Config {
        bool console_output = true;
        bool file_output = false;
        bool remote_logging = false;
        std::string log_format = "[%timestamp%] %level% %message%\n -> File: %file%:%line% (Function: %function%)\n";
        std::string log_filename = "error_log.txt"; // log filename
    };

    static inline void set_config(const Config& config) noexcept {
        log_config = config;
    }

    // asynchronous logging
    template <typename... Args>
    static void log(LogLevel level, const std::source_location location = std::source_location::current(), Args&&... args) {
        std::ostringstream message_stream;
        (message_stream << ... << std::forward<Args>(args));
        std::string log_message = format_message(message_stream.str(), level, location);
        
        std::async(std::launch::async, [log_message, level]() {
            if (log_config.console_output) {
                write_to_console(log_message, level);
            }
            if (log_config.file_output) {
                write_to_file(log_message);
            }
        });
    }

    // profiles function
    static void start_profiling(const std::string_view tag) noexcept {
        std::lock_guard<std::mutex> lock(log_mutex);
        profiling_data[tag] = std::chrono::high_resolution_clock::now();
    }

    static void end_profiling(const std::string_view tag) noexcept {
        std::lock_guard<std::mutex> lock(log_mutex);
        auto end_time = std::chrono::high_resolution_clock::now();
        if (profiling_data.find(tag) != profiling_data.end()) {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - profiling_data[tag]).count();
            log(LogLevel::Debug, std::source_location::current(), "Execution time for ", tag, ": ", duration, " microseconds");
            profiling_data.erase(tag);
        }
    }

    // profiles the function call
    template <typename Func, typename... Args>
    [[nodiscard]] static auto profile_function(const std::string_view tag, Func&& func, Args&&... args) noexcept {
        auto start_time = std::chrono::high_resolution_clock::now();
        auto result = std::forward<Func>(func)(std::forward<Args>(args)...);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        log(LogLevel::Debug, std::source_location::current(), "Execution time for ", tag, ": ", duration, " microseconds");
        return result;
    }

private:
    static std::mutex log_mutex;
    static Config log_config;
    static std::map<std::string, std::chrono::high_resolution_clock::time_point> profiling_data;

    // format log with the user defined format
    static std::string format_message(const std::string_view message, 
                                      LogLevel level, 
                                      const std::source_location location) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::ostringstream oss;
        std::string formatted_message = log_config.log_format;

        // replace placeholders
        formatted_message.replace(formatted_message.find("%timestamp%"), 10, std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S"));
        formatted_message.replace(formatted_message.find("%level%"), 7, get_log_level_prefix(level));
        formatted_message.replace(formatted_message.find("%message%"), 9, message);
        formatted_message.replace(formatted_message.find("%file%"), 6, location.file_name());
        formatted_message.replace(formatted_message.find("%line%"), 6, std::to_string(location.line()));
        formatted_message.replace(formatted_message.find("%function%"), 10, location.function_name());
        
        oss << formatted_message;
        return oss.str();
    }

    static std::string get_log_level_prefix(LogLevel level) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        switch (level) {
            case LogLevel::Info:
                SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
                return "[+]";
            case LogLevel::Warning:
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                return "[!]";
            case LogLevel::Error:
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
                return "[-]";
            case LogLevel::Debug:
                SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
                return "[*]";
            default:
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                return "[?]";
        }
    }

    static void write_to_console(const std::string& log_message, LogLevel level) {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cerr << log_message << std::flush;
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    static void write_to_file(const std::string& log_message) {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::ofstream log_file(log_config.log_filename, std::ios::app);
        if (log_file.is_open()) {
            log_file << log_message;
            log_file.flush();
        }
    }

    static inline std::mutex log_mutex;
    static inline Config log_config;
    static inline std::map<std::string, std::chrono::high_resolution_clock::time_point> profiling_data;
};

using LogLevel = Logger::LogLevel;
