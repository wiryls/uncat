#include <catch2/catch_test_macros.hpp>
#include <uncat/detail/types.hpp>

TEST_CASE("find_if", "[types]")
{
    using uncat::detail::pack;
    using uncat::detail::same_to;
    using uncat::detail::find_if;
    REQUIRE(find_if<pack, same_to<int>::template type, pack<int, void, double>>::value == true );
    REQUIRE(find_if<pack, same_to<int>::template type, pack<void, int, double>>::value == true );
    REQUIRE(find_if<pack, same_to<int>::template type, pack<char, double, int>>::value == true );
    REQUIRE(find_if<pack, same_to<int>::template type, pack<void, void, void >>::value == false);
    REQUIRE(find_if<pack, same_to<int>::template type, pack<     void, double>>::value == false);
    REQUIRE(find_if<pack, same_to<int>::template type, pack<              int>>::value == true );
    REQUIRE(find_if<pack, same_to<int>::template type, pack<                 >>::value == false);

    {
        auto lhs = find_if<pack, same_to<float>::template type, pack<char, int, float, void>>::type();
        auto rhs = double();
        REQUIRE(lhs == rhs);
    }
}

TEST_CASE("filter", "[types]")
{
    using uncat::detail::pack;
    using uncat::detail::same_to;
    using uncat::detail::filter;

    REQUIRE(std::is_same_v<filter<pack, same_to<int>::template type, pack<>, pack<int, void, int, double>>::type, pack<int, int>>);
}

TEST_CASE("is_subset", "[types]")
{
    using uncat::detail::pack;
    using uncat::detail::is_subset;
    REQUIRE(is_subset<pack, pack<double, void, int>, pack<int, void, double>>::value == true );
    REQUIRE(is_subset<pack, pack<int, void, double>, pack<int, void, double>>::value == true );
    REQUIRE(is_subset<pack, pack<int, void        >, pack<int, void, double>>::value == true );
    REQUIRE(is_subset<pack, pack<int, void, float >, pack<int, void, double>>::value == false);
    REQUIRE(is_subset<pack, pack<int, void, float >, pack<                 >>::value == false);
    REQUIRE(is_subset<pack, pack<     void        >, pack<int, void, double>>::value == true );
    REQUIRE(is_subset<pack, pack<                 >, pack<int, void, double>>::value == true );
    REQUIRE(is_subset<pack, pack<                 >, pack<                 >>::value == true );
}
