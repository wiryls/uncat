#include <catch2/catch_test_macros.hpp>
#include <uncat/types/types.hpp>

TEST_CASE("reverse", "[types]")
{
    using uncat::types::pack;
    using uncat::types::reverse;

    REQUIRE(std::is_same_v<typename reverse<pack<void, int, double>>::type, pack<double, int, void>>);
    REQUIRE(std::is_same_v<typename reverse<pack<void, void, void>>::type, pack<void, void, void>>);
    REQUIRE(std::is_same_v<typename reverse<pack<>>::type, pack<>>);
}

TEST_CASE("find_if", "[types]")
{
    using uncat::types::pack;
    using uncat::types::same;
    using uncat::types::find_if;
    REQUIRE(find_if<same<int>::type, pack<int, void, double>>::value == true );
    REQUIRE(find_if<same<int>::type, pack<void, int, double>>::value == true );
    REQUIRE(find_if<same<int>::type, pack<char, double, int>>::value == true );
    REQUIRE(find_if<same<int>::type, pack<void, void, void >>::value == false);
    REQUIRE(find_if<same<int>::type, pack<     void, double>>::value == false);
    REQUIRE(find_if<same<int>::type, pack<              int>>::value == true );
    REQUIRE(find_if<same<int>::type, pack<                 >>::value == false);

    {
        auto lhs = find_if<same<float>::type, pack<char, int, float, void>>::type();
        auto rhs = double();
        REQUIRE(lhs == rhs);
    }
}

TEST_CASE("filter", "[types]")
{
    using uncat::types::pack;
    using uncat::types::same;
    using uncat::types::filter;

    REQUIRE(std::is_same_v<filter<same<int>::type, pack<int, void, int, double>>::type, pack<int, int>>);
}

TEST_CASE("is_subset", "[types]")
{
    using uncat::types::pack;
    using uncat::types::is_subset;
    REQUIRE(is_subset<pack<double, void, int>, pack<int, void, double>>::value == true );
    REQUIRE(is_subset<pack<int, void, double>, pack<int, void, double>>::value == true );
    REQUIRE(is_subset<pack<int, void        >, pack<int, void, double>>::value == true );
    REQUIRE(is_subset<pack<int, void, float >, pack<int, void, double>>::value == false);
    REQUIRE(is_subset<pack<int, void, float >, pack<                 >>::value == false);
    REQUIRE(is_subset<pack<     void        >, pack<int, void, double>>::value == true );
    REQUIRE(is_subset<pack<                 >, pack<int, void, double>>::value == true );
    REQUIRE(is_subset<pack<                 >, pack<                 >>::value == true );
}

TEST_CASE("distinct", "[types]")
{
    using uncat::types::pack;
    using uncat::types::distinct;
    using uncat::types::distinct_stable;

    REQUIRE(std::is_same_v<typename distinct<pack<int, int, double>>::type, pack<int, double>>);
    REQUIRE(std::is_same_v<typename distinct<pack<int, double, int>>::type, pack<double, int>>);
    REQUIRE(std::is_same_v<typename distinct<pack<int, int, int, int>>::type, pack<int>>);
    REQUIRE(std::is_same_v<typename distinct<pack<int, double, double, int>>::type, pack<double, int>>);

    REQUIRE(std::is_same_v<typename distinct_stable<pack<int, int, double>>::type, pack<int, double>>);
    REQUIRE(std::is_same_v<typename distinct_stable<pack<int, double, int>>::type, pack<int, double>>);
    REQUIRE(std::is_same_v<typename distinct_stable<pack<int, int, int, int>>::type, pack<int>>);
    REQUIRE(std::is_same_v<typename distinct_stable<pack<int, double, double, int>>::type, pack<int, double>>);
}
