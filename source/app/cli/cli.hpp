#pragma once

/// Command-line interface for TestCpp.

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace testcpp::cli
{

/// Parsed command-line configuration.
struct Config
{
    /// Positional arguments (names to greet).
    std::vector<std::string_view> names;
    /// Whether --help was specified.
    bool help = false;
    /// Whether --version was specified.
    bool version = false;
    /// Whether --uppercase was specified.
    bool uppercase = false;
};

/// CLI parse error.
struct CliError
{
    /// Category of the error.
    enum class Kind : std::uint8_t
    {
        /// Unrecognized command-line option.
        unknown_option,
    };

    /// Error category.
    Kind kind;
    /// Contextual detail (e.g. the offending flag).
    std::string_view detail;
};

/// Formats a CliError into a human-readable message.
///
/// \param err the error to format
/// \return human-readable error string
[[nodiscard]] auto
format_error(const CliError &err) -> std::string;

/// Parses command-line arguments into a Config.
///
/// Handles --help/-h, --version/-v, --uppercase/-u.
/// Unknown flags produce a CliError.
///
/// \param args command-line arguments (including program name at args[0])
/// \return parsed Config, or CliError on failure
[[nodiscard]] auto
parse(std::span<const std::string_view> args)
    -> std::expected<Config, CliError>;

/// Executes the program logic based on parsed config.
///
/// \param config parsed command-line configuration
/// \return exit code (0 for success)
[[nodiscard]] auto
run(const Config &config) -> int;

} // namespace testcpp::cli
