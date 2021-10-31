#include <algorithm>
#include <vector>
#include <mutex>
#include <catch2/catch_test_macros.hpp>

#include <uncat/exec/executor.hpp>

TEST_CASE("executor(0) is dead silence", "[exec]")
{
    SECTION("push back experiment")
    {
        auto v = std::vector<int>();
        {
            auto ex = uncat::exec::executor(0);
            auto ok = ex([&v] { v.push_back(0); });
            REQUIRE(!ok);
        }
        REQUIRE(v.empty());
    }
}

TEST_CASE("executor(1) is strictly ordered", "[exec]")
{
    SECTION("push back experiment")
    {
        auto n = std::size_t(100);
        auto v = std::vector<std::size_t>();
        {
            auto ex = uncat::exec::executor(1);
            for (auto i = std::size_t(); i < n; ++i)
                ex([&v, i = i] { v.push_back(i); });
        }

        for (auto i = std::size_t(); i < n; ++i)
            REQUIRE(v[i] == i);
    }

    SECTION("with std::bind")
    {
        auto m = std::size_t(4);
        auto n = std::size_t(8);
        auto v = std::vector<std::size_t>();
        {
            auto xs = std::vector<std::size_t>{ 0, 1, 2, 3, 4, 5, 6, 7 };
            auto ex = uncat::exec::executor(1);
            {
                using ctyp = std::vector<std::size_t>;
                using iter = ctyp::const_iterator;
                using bier = std::back_insert_iterator<ctyp>;
                for (auto i = std::size_t(); i < m; ++i)
                {
                    ex(std::bind
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
        for (auto i = std::size_t(); i < v.size(); ++i)
            REQUIRE(v[i] == (i % n));
    }
}

TEST_CASE("executor(n, n >= 2) is a mess", "[exec]")
{
    SECTION("push back experiment")
    {
        auto x = std::size_t(30);
        auto y = std::size_t(30);
        auto v = std::vector<std::size_t>();
        auto m = std::mutex();
        {
            auto ex = uncat::exec::executor(8);
            for (auto i = std::size_t(); i < x * y; i += x)
            {
                auto l = i;
                auto r = i + x;
                ex([&v, &m, l, r]
                {
                    std::scoped_lock _(m);
                    for (auto i = l; i < r; ++i)
                        v.push_back(i);
                });
            }
        }

        std::sort(v.begin(), v.end());
        for (auto i = std::size_t(); i < x * y; ++i)
            REQUIRE(v[i] == i);
    }
}
