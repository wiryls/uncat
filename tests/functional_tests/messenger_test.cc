#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include <uncat/messenger/messenger.h>

TEST_CASE("lambda handlers", "[messenger]")
{
    using uncat::messenger;
    auto m = messenger<std::string, std::size_t, float, std::string>();

    static_assert(uncat::types::oneof<std::size_t, std::size_t, float, std::string>);

    SECTION("primitives")
    {
        auto n = std::size_t(10);
        auto v = std::vector<std::size_t>();
        m.add_handler<std::size_t>("collector", [&](std::size_t i) { v.push_back(i); });

        for (auto i = std::size_t(); i < n; ++i)
            m.send(std::move(i));

        m.remove_handler<std::size_t>("collector").wait();

        for (auto i = std::size_t(); i < n; ++i)
            REQUIRE(v[i] == i);
    }

    SECTION("std::string")
    {
        auto n = std::size_t(10);
        auto v = std::vector<std::string>();
        m.add_handler<std::string>("collector", [&](std::string const & s) { v.push_back(s); });

        for (auto i = std::size_t(); i < n; ++i)
        {
            if (i % 2 == 1)
            {
                auto s = std::to_string(i);
                m.send(s); // &
            }
            else
            {
                m.send(std::to_string(i)); // &&
            }
        }

        m.remove_handler<std::size_t>("collector").wait();

        for (auto i = std::size_t(); i < n; ++i)
            REQUIRE(v[i] == std::to_string(i));
    }
}
