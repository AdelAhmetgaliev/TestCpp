/// Tests for the greeter demo module.

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>

#include "greeter.hpp"

TEST_CASE("greet builds a greeting", "[greeter]") {
    SECTION("uses the given name") {
        CHECK(testcpp::greet("World") == "Hello, World!");
    }

    SECTION("handles an empty name") {
        CHECK(testcpp::greet("") == "Hello, !");
    }

    SECTION("returns a std::string") {
        CHECK(std::is_same_v<decltype(testcpp::greet("x")), std::string>);
    }
}
