#include <type_traits>

template <typename I> struct quicksort
{
private:
    template <I... xs> struct list
    {
        using to_integer_sequence = std::integer_sequence<I, xs...>;
    };

    template <typename T, typename R> struct join
    {};

    template <I... ls, I... rs> struct join<list<ls...>, list<rs...>>
    {
        using type = list<ls..., rs...>;
    };

    template <I y> struct compare
    {
        template <I x> struct less
        {
            auto static constexpr value = x < y;
        };

        template <I x> struct not_less
        {
            auto static constexpr value = x >= y;
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
        using type  = typename join<typename join<left, list<x>>::type, right>::type;
    };

public:
    template <I... xs> using apply = typename sort<list<xs...>>::type::to_integer_sequence;
};

template <typename I, I... xs> using quicksort_t = typename quicksort<I>::template apply<xs...>;
