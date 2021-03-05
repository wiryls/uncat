#include <vector>
#include <string>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <uncat/messenger.hpp>

TEST_CASE("notify with push back", "[messenger]")
{
    using uncat::messenger;
    auto v = std::vector<int>();
    auto m = messenger<std::string, int>();

    v.reserve(10000);

    BENCHMARK("10000 notify") {
        m.add_handler<int>("collector", [&](int i) { v.push_back(i); });
        
        for (auto i = 0; i < 10000; ++i)
            m.send(i);

        return m.remove_handler<int, true>("collector");
    };
}
