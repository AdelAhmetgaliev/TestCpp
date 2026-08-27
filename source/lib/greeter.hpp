#pragma once

/// Greeter demo module (template scaffolding): builds a greeting string.

#include <string>
#include <string_view>

namespace testcpp {

/// Builds a greeting message for the given name.
///
/// \param name person or thing to greet
/// \return "Hello, <name>!"
[[nodiscard]] auto greet(std::string_view name) -> std::string;

} // namespace testcpp
