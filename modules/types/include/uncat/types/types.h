#pragma once
#include <type_traits>

namespace uncat::types
{

/// curry
template <template <typename...> class F, typename... H> struct curry
{
    template <typename... T> using type = F<H..., T...>;
};

template <template <typename...> class F, typename... T> struct append
{
    template <typename... H> using type = F<H..., T...>;
};

/// make a non variadic template from a variadic template
template <unsigned, template <typename...> class T> struct fix
{};

template <template <typename...> class T> struct fix<0, T>
{
    template <typename U> using type = typename T<U>::type;
};

template <template <typename...> class T> struct fix<1, T>
{
    template <typename U> using type = typename T<U>::type;
};

template <template <typename...> class T> struct fix<2, T>
{
    template <typename A, typename B> using type = typename T<A, B>::type;
};

} // namespace uncat::types

namespace uncat::types
{

/// same partially fills a std::is_same with T.
template <typename T> using same = fix<1, curry<std::is_same, T>::template type>;

} // namespace uncat::types

namespace uncat::types
{
/////////////////////////////////////////////////////////////////////////
// types -> type

/// a...b -> a
template <typename H, typename... T> struct first
{
    using type = H;
};

template <typename H, typename... T> using first_t = typename first<H, T...>::type;

/// a...b -> b
template <typename H, typename... T> struct last
{
    using type = typename last<T...>::type;
};

template <typename H> struct last<H>
{
    using type = H;
};

template <typename H, typename... T> using last_t = typename types::last<H, T...>::type;

/// f -> g a -> f a
template <template <typename...> class M, typename C> struct map;

template <template <typename...> class F, template <typename...> class C, typename... T> struct map<F, C<T...>>
{
    using type = F<T...>;
};

template <template <typename...> class M, typename C>
requires requires { typename map<M, C>::type; }
using map_t = typename map<M, C>::type;

/// a -> ... a ... -> bool
template <typename T, typename... U> struct exists : std::bool_constant<(std::is_same_v<T, U> || ...)>
{};

template <typename T, typename... U> auto inline constexpr exists_v = exists<T, U...>::value;

/// (a -> bool) -> a... -> bool
template <template <typename> class P, typename... T> struct any : std::bool_constant<(P<T>::value || ...)>
{};

template <template <typename> class P, typename... T> auto inline constexpr any_v = any<P, T...>::value;

/// (a -> bool) -> a... -> a
template <
    template <typename> class P, // unary predicate
    typename... T>               // type list
struct find;

template <template <typename> class P, typename H, typename... T>
requires(P<H>::value == false && any_v<P, T...>)
struct find<P, H, T...>
{
    using type = typename find<P, T...>::type;
};

template <template <typename> class P, typename H, typename... T>
requires(P<H>::value)
struct find<P, H, T...>
{
    using type = H;
};

template <
    template <typename> class P, // unary predicate
    typename... T>               // type list
requires requires { typename find<P, T...>::type; }
using find_t = typename find<P, T...>::type;

} // namespace uncat::types

namespace uncat::types
{
/////////////////////////////////////////////////////////////////////////
// types -> types

/// a helper to pack types into a type.
template <typename... T> struct pack
{};

/// [a] -> [b] -> [a, b]
template <typename L, typename R> struct join;

template <template <typename...> class C, typename... L, typename... R> struct join<C<L...>, C<R...>>
{
    using type = C<L..., R...>;
};

template <typename L, typename R>
requires requires { typename join<L, R>::type; }
using join_t = typename join<L, R>::type;

/// [a...b] -> [b...a]
template <typename C> struct reverse
{};

template <template <typename...> class C, typename H, typename... T> struct reverse<C<H, T...>>
{
    using type = join_t<typename reverse<C<T...>>::type, C<H>>;
};

template <template <typename...> class C> struct reverse<C<>>
{
    using type = C<>;
};

template <typename C>
requires requires { typename reverse<C>::type; }
using reverse_t = typename reverse<C>::type;

/// a -> [...] -> bool
template <typename T, typename C> struct is_in : std::false_type
{};

template <typename T, template <typename...> class C, typename... U> struct is_in<T, C<U...>> : types::exists<T, U...>
{};

template <typename T, typename C> auto inline constexpr is_in_v = is_in<T, C>::value;

/// f -> [...] -> [..]
template <
    template <typename> class P, // unary predicate
    typename C>                  // something like C<U...>
struct filter;

template <
    template <typename> class P,    // predicate
    template <typename...> class C, // container
    typename H,                     // head
    typename... T>                  // tail
struct filter<P, C<H, T...>>
{
    using type = join_t<std::conditional_t<P<H>::value, C<H>, C<>>, typename filter<P, C<T...>>::type>;
};

template <template <typename> class P, template <typename...> class C> struct filter<P, C<>>
{
    using type = C<>;
};

template <template <typename> class P, typename C> using filter_t = typename filter<P, C>::type;

/// check if L is a subset of R.
template <typename L, typename R> struct is_subset;

template <template <typename...> class C, typename... L, typename... R>
struct is_subset<C<L...>, C<R...>> : std::bool_constant<(exists<L, R...>::value && ...)>
{};

template <typename L, typename R>
requires requires { is_subset<L, R>::value; }
auto inline constexpr is_subset_v = is_subset<L, R>::value;

/// distinct
template <typename C> struct distinct;

template <template <typename...> class C, typename H, typename... T> struct distinct<C<H, T...>>
{
    using type = join_t<std::conditional_t<exists<H, T...>::value, C<>, C<H>>, typename distinct<C<T...>>::type>;
};

template <template <typename...> class C> struct distinct<C<>>
{
    using type = C<>;
};

template <typename C>
requires requires { typename distinct<C>::type; }
using distinct_t = typename distinct<C>::type;

/// stable version
template <typename C>
requires requires { typename reverse_t<C>; }
using distinct_stable_t = reverse_t<distinct_t<reverse_t<C>>>;

} // namespace uncat::types
