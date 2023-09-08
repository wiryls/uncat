#include <type_traits>

template <typename I> struct compare
{
    template <I l, I r> struct less
    {
        auto static constexpr value = l < r;
    };
};

template <typename I, template <I, I> class C = compare<I>::template less> struct quicksort
{
private:
    template <I... xs> struct list
    {
        using to_integer_sequence = std::integer_sequence<I, xs...>;
    };

    template <typename H, typename... T> struct join
    {
        using type = H;
    };

    template <typename F, typename S, typename... T> struct join<F, S, T...>
    {
        using type = typename join<typename join<F, S>::type, T...>::type;
    };

    template <template <I...> class T, I... ls, I... rs> struct join<T<ls...>, T<rs...>>
    {
        using type = T<ls..., rs...>;
    };

    template <I y> struct compare
    {
        template <I x> struct less
        {
            auto static constexpr value = C<x, y>::value;
        };

        template <I x> struct not_less
        {
            auto static constexpr value = !C<x, y>::value;
        };
    };

    template <template <I> class P, I... xs> struct filter
    {
        using type = list<>;
    };

    template <template <I> class P, I x, I... xs> struct filter<P, x, xs...>
    {
        using type = typename filter<P, xs...>::type;
    };

    template <template <I> class P, I x, I... xs>
    requires(P<x>::value)
    struct filter<P, x, xs...>
    {
        using type = typename join<list<x>, typename filter<P, xs...>::type>::type;
    };

    template <typename L> struct sort
    {};

    template <I... xs> struct sort<list<xs...>>
    {
        using type = list<>;
    };

    template <I x, I... xs> struct sort<list<x, xs...>>
    {
        using left  = typename sort<typename filter<compare<x>::template less, xs...>::type>::type;
        using right = typename sort<typename filter<compare<x>::template not_less, xs...>::type>::type;
        using type  = typename join<left, list<x>, right>::type;
    };

public:
    template <I... xs> using apply = typename sort<list<xs...>>::type::to_integer_sequence;
};

template <typename I, I... xs> using quicksort_t = typename quicksort<I>::template apply<xs...>;

template <typename I, template <I, I> class C = compare<I>::template less> struct mergesort
{
private:
    template <I... xs> struct list
    {
        template <I x> using push_front = list<x, xs...>;

        using to_integer_sequence = std::integer_sequence<I, xs...>;
    };

    template <std::size_t k, typename L, typename R = list<>> struct split
    {};

    template <I... ls, I... rs> struct split<0, list<ls...>, list<rs...>>
    {
        using left  = list<rs...>;
        using right = list<ls...>;
    };

    template <std::size_t k, I... ls, I l, I... rs>
    requires(k > 0)
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

    template <I... rs> struct merge<list<>, list<rs...>>
    {
        using type = list<rs...>;
    };

    template <I... ls> struct merge<list<ls...>, list<>>
    {
        using type = list<ls...>;
    };

    template <> struct merge<list<>, list<>>
    {
        using type = list<>;
    };

    template <typename L> struct sort
    {};

    template <I... xs> struct sort<list<xs...>>
    {
        using split = split<sizeof...(xs) / 2, list<xs...>>;
        using left  = typename sort<typename split::left>::type;
        using right = typename sort<typename split::right>::type;
        using type  = typename merge<left, right>::type;
    };

    template <I x, I y> struct sort<list<x, y>>
    {
        auto static constexpr compare = C<x, y>::value;
        auto static constexpr first   = compare ? x : y;
        auto static constexpr second  = compare ? y : x;

        using type = list<first, second>;
    };

    template <I x> struct sort<list<x>>
    {
        using type = list<x>;
    };

    template <> struct sort<list<>>
    {
        using type = list<>;
    };

public:
    template <I... xs> using apply = typename sort<list<xs...>>::type::to_integer_sequence;
};

template <typename I, I... xs> using mergesort_t = typename mergesort<I>::template apply<xs...>;
