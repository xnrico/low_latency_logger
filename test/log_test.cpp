#include "logger.hpp"

auto main(int argc, char** argv) -> int {
    ricox::logger logger{"test.log"};

    // Simulate some logging
    logger.log("Logging a float: % and a double: %\n", 3.14159f, 2.718281828459045);
    logger.log("Logging a C-string: '%' and a C++ string: '%'\n", "C-style string", std::string{"C++ string"});

    return 0;
}