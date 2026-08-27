/// Implementation of the greeter demo module.

#include "greeter.hpp"

#include <format>
#include <string>
#include <string_view>

namespace testcpp {

auto greet(std::string_view name) -> std::string {
    return std::format("Hello, {}!", name);
}

} // namespace testcpp
