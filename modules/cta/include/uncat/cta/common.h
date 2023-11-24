#pragma once

namespace uncat::cta
{

// Output something like std::integer_sequence.
template <typename O, template <typename X, X...> class T, typename I, I x, I... xs>
auto inline static operator<<(O & oss, T<I, x, xs...> /* unused */) -> O &
{
    oss << x;
    ((oss << ' ').operator<< /* avoid being binary operator */ (xs), ...);
    return oss;
}

template <typename O, template <typename X, X...> class T, typename I>
auto inline static operator<<(O & oss, T<I> /* unused */) -> O &
{
    return oss;
}

// An integer_sequence like List.
template <typename I, I... xs> struct list
{
    template <template <typename T, T...> class L> using to = L<I, xs...>;

    template <I x> using push_front = list<I, x, xs...>;
    template <I x> using push_back  = list<I, xs..., x>;
};

// Comparator \ less \ greater.
template <typename I, template <typename X, X l, X r> class C> struct comparator
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

} // namespace uncat::cta

namespace uncat::cta
{
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

} // namespace aux

template <typename... T> using join_t = typename aux::join<T...>::type;

} // namespace uncat::cta
