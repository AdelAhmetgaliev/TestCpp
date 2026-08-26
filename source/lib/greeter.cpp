#include "greeter.hpp"

#include <format>

namespace testcpp {

auto greet(std::string_view name) -> std::string {
    return std::format("Hello, {}!", name);
}

} // namespace testcpp
