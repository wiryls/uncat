#include <catch2/catch_test_macros.hpp>
#include <concepts>
#include <uncat/types/types.hpp>

TEST_CASE("exists, any", "[types]")
{
    using uncat::types::curry
        , uncat::types::append
        , uncat::types::same
        , uncat::types::exists
        , uncat::types::exists_v
        , uncat::types::any_v;

    static_assert(!exists_v<int, float, float, float>);
    static_assert( exists_v<int, float, int, double>);
    static_assert( exists_v<int, float, int>);

    static_assert(!any_v<same<int>::type>);
    static_assert(!any_v<same<int>::type, float>);
    static_assert( any_v<same<int>::type, float, int>);
    static_assert( any_v<append<exists, float, int, double>::type, float>);
}

TEST_CASE("reverse", "[types]")
{
    using uncat::types::pack
        , uncat::types::reverse
        , uncat::types::reverse_t;

    static_assert(std::same_as<reverse_t<pack<void, int, double>>, pack<double, int, void>>);
    static_assert(std::same_as<reverse_t<pack<void, void, void>>, pack<void, void, void>>);
    static_assert(std::same_as<reverse_t<pack<>>, pack<>>);
}

TEST_CASE("filter", "[types]")
{
    using uncat::types::pack
        , uncat::types::same
        , uncat::types::filter_t;

    static_assert(std::same_as<filter_t<same<int>::type, pack<int, void, int, double>>, pack<int, int>>);
}

TEST_CASE("find", "[types]")
{
    using uncat::types::pack
        , uncat::types::same
        , uncat::types::find_t;

    static_assert(sizeof find_t<same<int>::type, int, void, double>);
    static_assert(sizeof find_t<same<int>::type, void, int, double>);
    static_assert(sizeof find_t<same<int>::type, char, double, int>);
    static_assert(sizeof find_t<same<int>::type,               int>);
}

TEST_CASE("is_subset", "[types]")
{
    using uncat::types::pack
        , uncat::types::is_subset_v;

    static_assert( is_subset_v<pack<double, void, int>, pack<int, void, double>>);
    static_assert( is_subset_v<pack<int, void, double>, pack<int, void, double>>);
    static_assert( is_subset_v<pack<int, void        >, pack<int, void, double>>);
    static_assert(!is_subset_v<pack<int, void, float >, pack<int, void, double>>);
    static_assert(!is_subset_v<pack<int, void, float >, pack<                 >>);
    static_assert( is_subset_v<pack<     void        >, pack<int, void, double>>);
    static_assert( is_subset_v<pack<                 >, pack<int, void, double>>);
    static_assert( is_subset_v<pack<                 >, pack<                 >>);
}

TEST_CASE("distinct", "[types]")
{
    using uncat::types::pack
        , uncat::types::distinct_t
        , uncat::types::distinct_stable_t;

    static_assert(std::same_as<distinct_t<pack<int,    int, double     >>, pack<   int, double>>);
    static_assert(std::same_as<distinct_t<pack<int, double,    int     >>, pack<double, int   >>);
    static_assert(std::same_as<distinct_t<pack<int,    int,    int, int>>, pack<   int        >>);
    static_assert(std::same_as<distinct_t<pack<int, double, double, int>>, pack<double, int   >>);

    static_assert(std::same_as<distinct_stable_t<pack<int,    int, double     >>, pack<int, double>>);
    static_assert(std::same_as<distinct_stable_t<pack<int, double,    int     >>, pack<int, double>>);
    static_assert(std::same_as<distinct_stable_t<pack<int,    int,    int, int>>, pack<int        >>);
    static_assert(std::same_as<distinct_stable_t<pack<int, double, double, int>>, pack<int, double>>);
}
