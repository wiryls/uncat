#pragma once

namespace uncat::cta
{

// Output something like std::integer_sequence.
template <typename O, template <typename X, X...> class T, typename I, I x, I... xs>
auto static operator<<(O & oss, T<I, x, xs...> /* unused */) -> O &
{
    return oss << x, ((oss << ' ').operator<< /* become unary right fold */ (xs), ...);
}

template <typename O, template <typename X, X...> class T, typename I>
auto static operator<<(O & oss, T<I> /* unused */) -> O &
{
    return oss;
}

template <typename O, template <auto...> class T, auto x, auto... xs>
auto static operator<<(O & oss, T<x, xs...> /* unused */) -> O &
{
    return oss << x, ((oss << ' ').operator<<(xs), ...);
}

template <typename O, template <auto...> class T> auto static operator<<(O & oss, T<> /* unused */) -> O &
{
    return oss;
}

namespace aux
{

template <auto... xs> struct typed_sequence_converter
{};

template <auto x, decltype(x)... xs> struct typed_sequence_converter<x, xs...>
{
    template <template <typename U, U...> class V> using to_typed = V<decltype(x), x, xs...>;
};

} // namespace aux

// An integer_sequence like List.
template <auto... xs> struct list : aux::typed_sequence_converter<xs...>
{
    template <template <auto...> class T> using to = T<xs...>;

    template <auto x> using push_front = list<x, xs...>;
    template <auto x> using push_back  = list<xs..., x>;
};

// Comparator less and greater.
template <auto l, decltype(l) r> struct less
{
    auto static constexpr value = l < r;
};

template <auto l, decltype(l) r> struct greater
{
    auto static constexpr value = l > r;
};

namespace aux
{

// Join lists like type.
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

template <template <auto...> class T, auto... ls, auto... rs> struct join<T<ls...>, T<rs...>>
{
    using type = T<ls..., rs...>;
};

template <template <auto...> class T, auto... ls, auto... ms, auto... rs> struct join<T<ls...>, T<ms...>, T<rs...>>
{
    using type = T<ls..., ms..., rs...>;
};

} // namespace aux

template <typename... T> using join_t = typename aux::join<T...>::type;

} // namespace uncat::cta
