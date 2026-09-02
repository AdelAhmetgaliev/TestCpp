/// Application entry point.

#include "main.hpp"

#include "cli.hpp"

auto
main(
    int argc,
    char *argv[]
) -> int
{
    const auto raw = std::span{argv, static_cast<std::size_t>(argc)};
    const auto args = std::vector<std::string_view>{raw.begin(), raw.end()};

    const auto config = testcpp::cli::parse(args);
    if (!config)
    {
        std::println(
            stderr, "{}\nUse --help for usage.",
            testcpp::cli::format_error(config.error())
        );
        return EXIT_FAILURE;
    }

    return testcpp::cli::run(*config);
}
