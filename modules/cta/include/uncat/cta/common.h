#pragma once

#include <utility>

namespace uncat {

// Output std::integer_sequence.
template <typename O, template <typename X, X...> class T, typename I, I... xs>
auto inline static operator<<(O & oss, T<I, xs...>) -> O &
{
    return (oss << ... << xs);
}

template <typename O, template <typename X, X...> class T, typename I, I x, I y, I... xs>
auto inline static operator<<(O & oss, T<I, x, y, xs...>) -> O &
{
    return oss << x << ", " << T<I, y, xs...>();
}

// Comparator \ less \ greater.
template <typename I, template <typename I, I l, I r> class C> struct comparator
{
    template <I l, I r> struct type
    {
        auto static constexpr value = C<I, l, r>::value;
    };
};

template <typename I, I l, I r> struct less
{
    auto static constexpr value = l < r;
};

template <typename I, I l, I r> struct greater
{
    auto static constexpr value = l > r;
};

} // namespace uncat

namespace uncat { namespace cta { namespace aux {

// Join lists type.
template <typename H, typename... T> struct join
{
    using type = H;
};

template <typename F, typename S, typename... T> struct join<F, S, T...>
{
    using type = typename join<typename join<F, S>::type, T...>::type;
};

template <template <typename...> class T, typename... L, typename... R> struct join<T<L...>, T<R...>>
{
    using type = T<L..., R...>;
};

template <template <typename X, X...> class T, typename I, I... ls, I... rs> struct join<T<I, ls...>, T<I, rs...>>
{
    using type = T<I, ls..., rs...>;
};

template <template <typename X, X...> class T, typename I, I... ls, I... ms, I... rs>
struct join<T<I, ls...>, T<I, ms...>, T<I, rs...>>
{
    using type = T<I, ls..., ms..., rs...>;
};

template <typename... T> using join_t = typename join<T...>::type;

// An integer_sequence like List.
template <typename I, I... xs> struct list
{
    using to_integer_sequence = std::integer_sequence<I, xs...>;

    template <I x> using push_front = list<I, x, xs...>;
    template <I x> using push_back  = list<I, xs..., x>;
};

template <typename Source> struct make_list
{};

template <template <typename X, X...> class T, typename I, I... xs> struct make_list<T<I, xs...>>
{
    using type = list<I, xs...>;
};

template <typename T> using make_list_t = typename make_list<T>::type;

}}} // namespace uncat::cta::aux
