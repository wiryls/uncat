#include <catch2/catch_test_macros.hpp>
#include <vt/detail/type.hpp>
#include <vt/detail/list.hpp>

TEST_CASE("find_if", "[list]")
{
    using vt::detail::pack;
    using vt::detail::same_to_type;
    using vt::detail::find_if_base;
    REQUIRE(find_if_base<pack, same_to_type<int>::template type, pack<int, void, double>>::value == true );
    REQUIRE(find_if_base<pack, same_to_type<int>::template type, pack<void, int, double>>::value == true );
    REQUIRE(find_if_base<pack, same_to_type<int>::template type, pack<char, double, int>>::value == true );
    REQUIRE(find_if_base<pack, same_to_type<int>::template type, pack<void, void, void >>::value == false);
    REQUIRE(find_if_base<pack, same_to_type<int>::template type, pack<     void, double>>::value == false);
    REQUIRE(find_if_base<pack, same_to_type<int>::template type, pack<              int>>::value == true );
    REQUIRE(find_if_base<pack, same_to_type<int>::template type, pack<                 >>::value == false);
}

TEST_CASE("filter_base", "[list]")
{
    using vt::detail::pack;
    using vt::detail::same_to_type;
    using vt::detail::filter_base;

    REQUIRE(std::is_same_v<filter_base<pack, same_to_type<int>::template type, pack<>, pack<int, void, int, double>>::type, pack<int, int>>);
}

TEST_CASE("is_subset_base", "[list]")
{
    using vt::detail::pack;
    using vt::detail::is_subset_base;
    REQUIRE(is_subset_base<pack, pack<double, void, int>, pack<int, void, double>>::value == true );
    REQUIRE(is_subset_base<pack, pack<int, void, double>, pack<int, void, double>>::value == true );
    REQUIRE(is_subset_base<pack, pack<int, void        >, pack<int, void, double>>::value == true );
    REQUIRE(is_subset_base<pack, pack<int, void, float >, pack<int, void, double>>::value == false);
    REQUIRE(is_subset_base<pack, pack<int, void, float >, pack<                 >>::value == false);
    REQUIRE(is_subset_base<pack, pack<     void        >, pack<int, void, double>>::value == true );
    REQUIRE(is_subset_base<pack, pack<                 >, pack<int, void, double>>::value == true );
    REQUIRE(is_subset_base<pack, pack<                 >, pack<                 >>::value == true );
}
