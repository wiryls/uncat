#include <algorithm>
#include <vector>
#include <mutex>
#include <catch2/catch_test_macros.hpp>

#include <pw/world_line.hpp>

TEST_CASE("world_line(0) is dead silence", "[world_line]")
{
    SECTION("push back experiment")
    {
        auto v = std::vector<int>();
        {
            auto ws = pw::world_line(0);
            ws.enqueue([&v] { v.push_back(0); });
            ws.enqueue([&v] { v.push_back(1); });
        }
        REQUIRE(v.empty());
    }
}

TEST_CASE("world_line(1) is strictly ordered", "[world_line]")
{
    SECTION("push back experiment")
    {
        auto n = 100;
        auto v = std::vector<int>();
        {
            auto ws = pw::world_line(1);
            for (auto i = 0; i < n; ++i)
                ws.enqueue([&v, i = i] { v.push_back(i); });
        }

        for (auto i = 0; i < n; ++i)
            REQUIRE(v[i] == i);
    }

    SECTION("with std::bind")
    {
        auto m = 4;
        auto n = 8;
        auto v = std::vector<int>();
        {
            auto xs = std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 };
            auto ws = pw::world_line(1);
            {
                using ctyp = std::vector<int>;
                using iter = ctyp::const_iterator;
                using bier = std::back_insert_iterator<ctyp>;
                for (auto i = 0; i < m; ++i)
                {
                    ws.enqueue(std::bind
                    (
                        std::copy<iter, bier>,
                        xs.begin(),
                        xs.end(),
                        std::back_inserter(v)
                    ));
                }
            }
        }

        REQUIRE(v.size() == m * n);
        for (auto i = 0; i < v.size(); ++i)
            REQUIRE(v[i] == (i % n));
    }
}

TEST_CASE("world_line(n, n >= 2) is a mess", "[world_line]")
{
    SECTION("push back experiment")
    {
        auto x = 30;
        auto y = 30;
        auto v = std::vector<int>();
        auto m = std::mutex();
        {
            auto ws = pw::world_line(8);
            for (auto i = 0; i < x * y; i += x)
            {
                auto l = i;
                auto r = i + x;
                ws.enqueue([&v, &m, l, r]
                {
                    std::lock_guard<std::mutex> _(m);
                    for (auto i = l; i < r; ++i)
                        v.push_back(i);
                });
            }
        }

        std::sort(v.begin(), v.end());
        for (auto i = 0; i < x * y; ++i)
            REQUIRE(v[i] == i);
    }
}
