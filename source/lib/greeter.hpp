#pragma once

#include <string>
#include <string_view>

namespace testcpp {

/// Builds a greeting message for the given name.
[[nodiscard]] auto greet(std::string_view name) -> std::string;

} // namespace testcpp
