#include "logger.hpp"
#include <thread>
#include <chrono>

auto main(int argc, char** argv) -> int {
    ricox::stdout_logger logger;
    ricox::file_logger file_logger{"mylog.txt"};

    // Simulate some logging
    logger.log("Logging a float: % and a double: %\n", 3.14159f, 2.718281828459045);
    logger.log("Logging a C-string: '%' and a C++ string: '%'\n", "C-style string", std::string{"C++ string"});

    file_logger.log("Logging a float: % and a double: %\n", 3.14159f, 2.718281828459045);
    file_logger.log("Logging a C-string: '%' and a C++ string: '%'\n", "C-style string", std::string{"C++ string"});

    // Give the loggers time to process the entries
    // std::this_thread::sleep_for(std::chrono::milliseconds{100});

    return 0;
}