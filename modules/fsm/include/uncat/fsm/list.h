#pragma once
#include <type_traits>

namespace uncat::fsm::aux
{

template <template <typename...> class T, typename... U> struct list_operations
{
    template <typename... V> using push_front            = T<V..., U...>;
    template <typename... V> using push_back             = T<U..., V...>;
    template <template <typename...> class V> using as   = V<U...>;
    template <typename V> auto static constexpr contains = (std::is_same_v<V, U> || ...);
};

template <typename... T> struct list : list_operations<list, T...>
{};

template <typename H, typename... T> struct list<H, T...> : list_operations<list, H, T...>
{
    using front = H;
};

template <typename L> struct distinct_stable
{};

template <template <typename...> class L, typename... V> struct distinct_stable<L<V...>>
{
private:
    template <typename O, typename... I> struct distinct_into
    {
        using type = O;
    };

    template <typename O, typename T, typename... I> struct distinct_into<O, T, I...>
    {
        using type = typename distinct_into<O, I...>::type;
    };

    template <typename... O, typename T, typename... I>
    requires(!(std::is_same_v<T, O> || ...))
    struct distinct_into<L<O...>, T, I...>
    {
        using type = typename distinct_into<L<O..., T>, I...>::type;
    };

public:
    using type = typename distinct_into<L<>, V...>::type;
};

} // namespace uncat::fsm::aux
