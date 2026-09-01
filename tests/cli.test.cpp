/// Tests for the CLI module.

#include <catch2/catch_test_macros.hpp>

#include <string_view>
#include <vector>

#include "cli.hpp"

TEST_CASE(
    "cli::parse handles --help",
    "[cli]"
)
{
    const auto args = std::vector<std::string_view>{"TestCpp", "--help"};
    const auto config = testcpp::cli::parse(args);
    REQUIRE(config.has_value());
    CHECK(config->help == true);
    CHECK(config->names.empty());
}

TEST_CASE(
    "cli::parse handles -h",
    "[cli]"
)
{
    const auto args = std::vector<std::string_view>{"TestCpp", "-h"};
    const auto config = testcpp::cli::parse(args);
    REQUIRE(config.has_value());
    CHECK(config->help == true);
}

TEST_CASE(
    "cli::parse handles --version",
    "[cli]"
)
{
    const auto args = std::vector<std::string_view>{"TestCpp", "--version"};
    const auto config = testcpp::cli::parse(args);
    REQUIRE(config.has_value());
    CHECK(config->version == true);
    CHECK(config->names.empty());
}

TEST_CASE(
    "cli::parse handles -v",
    "[cli]"
)
{
    const auto args = std::vector<std::string_view>{"TestCpp", "-v"};
    const auto config = testcpp::cli::parse(args);
    REQUIRE(config.has_value());
    CHECK(config->version == true);
}

TEST_CASE(
    "cli::parse handles --uppercase",
    "[cli]"
)
{
    const auto args = std::vector<std::string_view>{"TestCpp", "--uppercase"};
    const auto config = testcpp::cli::parse(args);
    REQUIRE(config.has_value());
    CHECK(config->uppercase == true);
    CHECK(config->names.empty());
}

TEST_CASE(
    "cli::parse handles -u",
    "[cli]"
)
{
    const auto args = std::vector<std::string_view>{"TestCpp", "-u"};
    const auto config = testcpp::cli::parse(args);
    REQUIRE(config.has_value());
    CHECK(config->uppercase == true);
}

TEST_CASE(
    "cli::parse handles positional names",
    "[cli]"
)
{
    const auto args = std::vector<std::string_view>{"TestCpp", "Alice", "Bob"};
    const auto config = testcpp::cli::parse(args);
    REQUIRE(config.has_value());
    CHECK(config->names.size() == 2);
    CHECK(config->names[0] == "Alice");
    CHECK(config->names[1] == "Bob");
}

TEST_CASE(
    "cli::parse handles mixed flags and names",
    "[cli]"
)
{
    const auto args =
        std::vector<std::string_view>{"TestCpp", "--uppercase", "World"};
    const auto config = testcpp::cli::parse(args);
    REQUIRE(config.has_value());
    CHECK(config->uppercase == true);
    CHECK(config->names.size() == 1);
    CHECK(config->names[0] == "World");
}

TEST_CASE(
    "cli::parse handles no arguments",
    "[cli]"
)
{
    const auto args = std::vector<std::string_view>{"TestCpp"};
    const auto config = testcpp::cli::parse(args);
    REQUIRE(config.has_value());
    CHECK(config->help == false);
    CHECK(config->version == false);
    CHECK(config->uppercase == false);
    CHECK(config->names.empty());
}

TEST_CASE(
    "cli::parse returns error on unknown flag",
    "[cli]"
)
{
    const auto args = std::vector<std::string_view>{"TestCpp", "--bad-flag"};
    const auto result = testcpp::cli::parse(args);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == testcpp::cli::CliError::Kind::unknown_option);
    CHECK(result.error().detail == "--bad-flag");
}

TEST_CASE(
    "cli::format_error formats unknown option",
    "[cli]"
)
{
    const auto err = testcpp::cli::CliError{
        .kind = testcpp::cli::CliError::Kind::unknown_option, .detail = "--foo"
    };
    CHECK(testcpp::cli::format_error(err) == "Unknown option: --foo");
}
