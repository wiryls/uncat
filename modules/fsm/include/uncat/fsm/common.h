#pragma once

namespace uncat::fsm::aux
{

template <typename T, typename U> auto constexpr same      = false;
template <typename T> auto constexpr same<T, T>            = true;
template <typename T, typename... U> auto constexpr one_of = (same<T, U> || ...);

template <typename...> struct optional
{};

template <typename T, typename... U> struct optional<T, U...>
{
    using type = T;
};

template <template <typename> class P, typename... I> struct find
{
    using type = optional<>;
};

template <template <typename> class P, typename T, typename... I>
requires(P<T>::value)
struct find<P, T, I...>
{
    using type = optional<T>;
};

template <template <typename> class P, typename T, typename... I>
requires(not P<T>::value)
struct find<P, T, I...>
{
    using type = typename find<P, I...>::type;
};

template <template <typename...> class T, typename... U> struct list_operations
{
    template <typename... V> using push_front            = T<V..., U...>;
    template <typename... V> using push_back             = T<U..., V...>;
    template <template <typename> class P> using find    = typename find<P, U...>::type;
    template <template <typename...> class V> using as   = V<U...>;
    template <typename V> auto static constexpr contains = one_of<V, U...>;
};

template <typename... T> struct list : list_operations<list, T...>
{};

template <typename H, typename... T> struct list<H, T...> : list_operations<list, H, T...>
{
    using first = H;
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
    requires(not one_of<T, O...>)
    struct distinct_into<L<O...>, T, I...>
    {
        using type = typename distinct_into<L<O..., T>, I...>::type;
    };

public:
    using type = typename distinct_into<L<>, V...>::type;
};

template <typename T>
concept type_member = requires { typename T::type; };

} // namespace uncat::fsm::aux
