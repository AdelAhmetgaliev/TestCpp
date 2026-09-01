/// Implementation of the command-line interface for TestCpp.

#include "cli.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <expected>
#include <format>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "greeter.hpp"

namespace testcpp::cli
{

namespace
{

/// Prints usage information to stdout.
void
print_usage()
{
    std::println("Usage: TestCpp [OPTIONS] [NAME...]");
    std::println("");
    std::println("Options:");
    std::println("  -h, --help       Show this help message");
    std::println("  -v, --version    Show version");
    std::println("  -u, --uppercase  Print greeting in UPPERCASE");
}

/// Prints version information to stdout.
void
print_version()
{
    std::println("TestCpp v{}", PROJECT_VERSION);
}

/// Checks whether an argument is a CLI flag (starts with '-').
auto
is_flag(
    std::string_view arg
) -> bool
{
    return arg.size() > 1 && arg[0] == '-';
}

/// Processes a single flag and updates config accordingly.
/// \return true if the flag was recognized, false otherwise
auto
handle_flag(
    std::string_view flag,
    Config &config
) -> bool
{
    if (flag == "-h" || flag == "--help")
    {
        config.help = true;
        return true;
    }
    if (flag == "-v" || flag == "--version")
    {
        config.version = true;
        return true;
    }
    if (flag == "-u" || flag == "--uppercase")
    {
        config.uppercase = true;
        return true;
    }
    return false;
}

} // anonymous namespace

auto
format_error(
    const CliError &err
) -> std::string
{
    switch (err.kind)
    {
    case CliError::Kind::unknown_option:
        return std::format("Unknown option: {}", err.detail);
    }
    return "Unknown error";
}

auto
parse(
    std::span<const std::string_view> args
) -> std::expected<Config, CliError>
{
    auto config = Config{};

    for (std::size_t i = 1; i < args.size(); ++i)
    {
        const auto &arg = args[i];

        if (is_flag(arg))
        {
            if (!handle_flag(arg, config))
            {
                return std::unexpected{CliError{
                    .kind = CliError::Kind::unknown_option,
                    .detail = arg,
                }};
            }
        }
        else
        {
            config.names.push_back(arg);
        }
    }

    return config;
}

auto
run(
    const Config &config
) -> int
{
    if (config.help)
    {
        print_usage();
        return 0;
    }

    if (config.version)
    {
        print_version();
        return 0;
    }

    const auto names = config.names.empty()
                           ? std::vector<std::string_view>{"World"}
                           : config.names;

    for (const auto &name : names)
    {
        auto greeting = testcpp::greet(name);

        if (config.uppercase)
        {
            std::ranges::transform(
                greeting, greeting.begin(),
                [](unsigned char c) -> int { return std::toupper(c); }
            );
        }

        std::println("{}", greeting);
    }

    return 0;
}

} // namespace testcpp::cli
