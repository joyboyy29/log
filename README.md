# Logger Library

## Overview

Logger is a lightweight C++ logging library that provides console and file logging, log level filtering, and performance profiling. It is designed for efficiency, thread safety, and ease of integration into existing projects. Writing this to practice writing *good* code.

## Features

- **Asynchronous Logging**: Non-blocking log writes using `std::async`.
- **Log Levels**: Supports `Info`, `Warning`, `Error`, and `Debug`.
- **Custom Log Formatting**: Define your own log message structure.
- **Colored Console Output**: Log level prefixes are color-coded so it looks good xD.
- **File Logging**: Logs can be saved to a configurable file.
- **Function Execution Profiling**: Measure execution time of code blocks and functions.

## Installation

Simply include `logger.hpp` in your project:

```cpp
#include "logger.hpp"
```

## Usage

### Basic Logging

```cpp
Logger::log(Logger::LogLevel::Info, "Start Success!");
Logger::log(Logger::LogLevel::Warning, "Running out of Memory!");
Logger::log(Logger::LogLevel::Error, "Failed to locate file");
Logger::log(Logger::LogLevel::Debug, "Value of Var test = ", 0xBADC0DE);
```

### Customizing Log Output

```cpp
Logger::Config config;
config.console_output = true;
config.file_output = true;
config.log_filename = "my_log.txt";
config.log_format = "[%timestamp%] %level% %message% [File: %file% | Line: %line%]";
Logger::set_config(config);
```

### Profiling Execution Time

```cpp
Logger::start_profiling("Test");
// Some work
std::this_thread::sleep_for(std::chrono::milliseconds(100));
Logger::end_profiling("Test");
```

### Profiling a Function Call (Works for any callable)

```cpp
int result = Logger::profile_function("Test2", []() {
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    return 510;
});
Logger::log(Logger::LogLevel::Info, "Test2 result: ", result);
```

## Configuration Options

| Option           | Type     | Default Value   | Description                         |
| ---------------- | -------- | --------------- | ----------------------------------- |
| `console_output` | `bool`   | `true`          | Enables console logging             |
| `file_output`    | `bool`   | `false`         | Enables file logging                |
| `remote_logging` | `bool`   | `false`         | Reserved for future network logging |
| `log_format`     | `string` | Customizable    | Define log structure                |
| `log_filename`   | `string` | `error_log.txt` | Output file name                    |

## License

This project is open-source and available for use in any project. Contributions and improvements are welcome!
