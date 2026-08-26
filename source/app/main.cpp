#include <print>

#include "greeter.hpp"

auto main() -> int {
    std::println("{} (v{})", testcpp::greet("World"), PROJECT_VERSION);
}
