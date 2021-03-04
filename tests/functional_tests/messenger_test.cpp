#include <vector>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <uncat/messenger.hpp>

TEST_CASE("lambda handlers", "[messenger]")
{
    using uncat::messenger;
    auto m = messenger<std::string, std::size_t, float>();

    SECTION("primitives")
    {
        auto n = std::size_t(10);
        auto v = std::vector<std::size_t>();
        m.add_handler<std::size_t>("collector", [&](std::size_t i) { v.push_back(i); });

        for (auto i = std::size_t(); i < n; ++i)
            m.send(i);

        auto o = m.remove_handler<std::size_t, true>("collector");
        REQUIRE(o);

        for (auto i = std::size_t(); i < n; ++i)
            REQUIRE(v[i] == i);
    }
}
