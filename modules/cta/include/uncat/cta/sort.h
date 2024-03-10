#pragma once
#include <uncat/cta/common.h>

namespace uncat::cta::aux
{

template <template <auto, auto> class C> struct quick_sort
{
private:
    template <auto y> struct partition
    {
        template <typename Source, typename Left = list<>, typename Right = list<>> struct on
        {
            using left  = Left;
            using right = Right;
        };

        template <auto x, auto... xs, auto... ls, typename R>
        requires(!C<y, x>::value)
        struct on<list<x, xs...>, list<ls...>, R> : on<list<xs...>, list<x, ls...>, R>
        {};

        template <auto x, auto... xs, typename L, auto... rs>
        requires C<y, x>::value
        struct on<list<x, xs...>, L, list<rs...>> : on<list<xs...>, L, list<x, rs...>>
        {};
    };

    template <typename Input> struct sort
    {
        using type = list<>;
    };

    template <auto x, auto... xs> struct sort<list<x, xs...>>
    {
        using partitioned  = partition<x>::template on<list<xs...>>;
        using left_branch  = typename sort<typename partitioned::left>::type;
        using right_branch = typename sort<typename partitioned::right>::type;
        using type         = join_t<left_branch, list<x>, right_branch>;
    };

public:
    template <auto... xs> using apply = typename sort<list<xs...>>::type;
};

template <template <auto, auto> class C> struct merge_sort
{
private:
    template <auto k, typename L, typename R = list<>> struct split
    {
        using left  = L;
        using right = R;
    };

    template <auto k, auto... ls, auto l, auto... rs>
    requires(k != 0)
    struct split<k, list<l, ls...>, list<rs...>>
    {
        using next  = split<k - 1, list<ls...>, list<rs..., l>>;
        using left  = typename next::left;
        using right = typename next::right;
    };

    template <typename L, typename R> struct merge
    {};

    template <auto l, auto... ls, auto r, auto... rs>
    requires(C<l, r>::value)
    struct merge<list<l, ls...>, list<r, rs...>>
    {
        using type = typename merge<list<ls...>, list<r, rs...>>::type::template push_front<l>;
    };

    template <auto l, auto... ls, auto r, auto... rs> struct merge<list<l, ls...>, list<r, rs...>>
    {
        using type = typename merge<list<l, ls...>, list<rs...>>::type::template push_front<r>;
    };

    template <typename R> struct merge<list<>, R>
    {
        using type = R;
    };

    template <typename L> struct merge<L, list<>>
    {
        using type = L;
    };

    template <typename L> struct sort
    {
        using type = list<>;
    };

    template <auto... xs> struct sort<list<xs...>>
    {
        using branch = split<sizeof...(xs) / 2, list<xs...>>;
        using left   = typename sort<typename branch::left>::type;
        using right  = typename sort<typename branch::right>::type;
        using type   = typename merge<left, right>::type;
    };

    template <auto x, auto y> struct sort<list<x, y>>
    {
        auto static constexpr first  = C<x, y>::value ? x : y;
        auto static constexpr second = C<x, y>::value ? y : x;

        using type = list<first, second>;
    };

    template <auto x> struct sort<list<x>>
    {
        using type = list<x>;
    };

public:
    template <auto... xs> using apply = typename sort<list<xs...>>::type;
};

} // namespace uncat::cta::aux

namespace uncat
{

template <typename Sorter, typename List> struct sort
{};

template <typename Sorter, template <auto...> class T, auto... xs> struct sort<Sorter, T<xs...>>
{
    using type = typename Sorter::template apply<xs...>::template to<T>;
};

template <typename Sorter, template <typename U, U...> class T, typename U, auto... xs> struct sort<Sorter, T<U, xs...>>
{
    using type = typename Sorter::template apply<xs...>::template to_typed<T>;
};

template <typename List, template <auto, auto> class Comparator = cta::less>
using quick_sort_t = typename sort<cta::aux::quick_sort<Comparator>, List>::type;

template <typename List, template <auto, auto> class Comparator = cta::less>
using merge_sort_t = typename sort<cta::aux::merge_sort<Comparator>, List>::type;

} // namespace uncat
