#pragma once

#include <uncat/cta/common.h>

namespace uncat::cta::aux
{

template <typename I, template <I, I> class C> struct quick_sort
{
private:
    template <I... xs> using list = list<I, xs...>;

    template <I y> struct partition
    {
        template <typename Source, typename Left = list<>, typename Right = list<>> struct on
        {
            using left  = Left;
            using right = Right;
        };

        template <I x, I... xs, I... ls, typename R>
        requires(!C<y, x>::value)
        struct on<list<x, xs...>, list<ls...>, R>
        {
            using next  = on<list<xs...>, list<x, ls...>, R>;
            using left  = typename next::left;
            using right = typename next::right;
        };

        template <I x, I... xs, typename L, I... rs>
        requires C<y, x>::value
        struct on<list<x, xs...>, L, list<rs...>>
        {
            using next  = on<list<xs...>, L, list<x, rs...>>;
            using left  = typename next::left;
            using right = typename next::right;
        };
    };

    template <typename Input> struct sort
    {
        using type = list<>;
    };

    template <I x, I... xs> struct sort<list<x, xs...>>
    {
        using partitioned  = partition<x>::template on<list<xs...>>;
        using left_branch  = typename sort<typename partitioned::left>::type;
        using right_branch = typename sort<typename partitioned::right>::type;
        using type         = join_t<left_branch, list<x>, right_branch>;
    };

public:
    template <I... xs> using apply = typename sort<list<xs...>>::type;
};

template <typename I, template <I, I> class C> struct merge_sort
{
private:
    template <I... xs> using list = list<I, xs...>;

    template <unsigned k, typename L, typename R = list<>> struct split
    {
        using left  = L;
        using right = R;
    };

    template <unsigned k, I... ls, I l, I... rs>
    requires(k != 0)
    struct split<k, list<l, ls...>, list<rs...>>
    {
        using next  = split<k - 1, list<ls...>, list<rs..., l>>;
        using left  = typename next::left;
        using right = typename next::right;
    };

    template <typename L, typename R> struct merge
    {};

    template <I l, I... ls, I r, I... rs>
    requires(C<l, r>::value)
    struct merge<list<l, ls...>, list<r, rs...>>
    {
        using type = typename merge<list<ls...>, list<r, rs...>>::type::template push_front<l>;
    };

    template <I l, I... ls, I r, I... rs> struct merge<list<l, ls...>, list<r, rs...>>
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

    template <I... xs> struct sort<list<xs...>>
    {
        using branch = split<sizeof...(xs) / 2, list<xs...>>;
        using left   = typename sort<typename branch::left>::type;
        using right  = typename sort<typename branch::right>::type;
        using type   = typename merge<left, right>::type;
    };

    template <I x, I y> struct sort<list<x, y>>
    {
        auto static constexpr first  = C<x, y>::value ? x : y;
        auto static constexpr second = C<x, y>::value ? y : x;

        using type = list<first, second>;
    };

    template <I x> struct sort<list<x>>
    {
        using type = list<x>;
    };

public:
    template <I... xs> using apply = typename sort<list<xs...>>::type;
};

} // namespace uncat::cta::aux

namespace uncat
{

template <typename T, template <typename I, I, I> class C = cta::less> struct quick_sort
{};

template <template <typename X, X...> class T, typename I, I... xs, template <typename X, X, X> class C>
struct quick_sort<T<I, xs...>, C>
{
    using type =
        typename cta::aux::quick_sort<I, cta::comparator<I, C>::template type>::template apply<xs...>::template to<T>;
};

template <typename T, template <typename I, I, I> class C = cta::less> struct merge_sort
{};

template <template <typename X, X...> class T, typename I, I... xs, template <typename X, X, X> class C>
struct merge_sort<T<I, xs...>, C>
{
    using type =
        typename cta::aux::merge_sort<I, cta::comparator<I, C>::template type>::template apply<xs...>::template to<T>;
};

template <typename List, template <typename I, I, I> class Comparator = cta::less>
using quick_sort_t = typename quick_sort<List, Comparator>::type;

template <typename List, template <typename I, I, I> class Comparator = cta::less>
using merge_sort_t = typename merge_sort<List, Comparator>::type;

} // namespace uncat
