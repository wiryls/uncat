#pragma once
#include <type_traits>

namespace uncat { namespace detail
{
    /////////////////////////////////////////////////////////////////////////
    // limited types -> type / value

    /// remove const reference of a type
    template
        < typename T
        > using remove_cvr_t = std::remove_cv_t<std::remove_reference_t<T>>;

    /// select_type returns L if C else R
    template
        < bool     C
        , typename L
        , typename R
        > struct select
    {
        using type = R;
    };

    template
        < typename L
        , typename R
        > struct select<true, L, R>
    {
        using type = L;
    };

    /// curry
    template
        < template<typename ...> class T
        , typename                 ... U
        > struct curry
    {
        template
            < typename ...V
            > using type = T<U..., V...>;
    };

    /// make a non variadic template from a variadic template
    template
        < template<typename ...> class T
        , std::size_t                  N = 0
        > struct non_variadic
    {
        template
            < typename ...U
            > using type = typename T<U...>::type;
    };

    template
        < template<typename ...> class T
        > struct non_variadic<T, 1>
    {
        template
            < typename V
            > using type = typename T<V>::type;
    };

    /// same_to partially fills a std::is_same with T.
    template
        < typename T
        > using same = non_variadic<curry<std::is_same, T>::template type, 1>;

    /// make sure the functor F could be invoked like F()(T()...) -> R
    template
        < typename F
        , typename R
        , typename ...T
        > struct functor_pass
    {
        bool constexpr static value = false;
    };

    template
        < typename F
        , typename ...T
        > struct functor_pass
            < F
            , decltype(std::declval<F>()(std::declval<T>() ...))
            , T...
            >
    {
        using type = F;
        bool constexpr static value = true;
    };

    /// 
}}

namespace uncat { namespace detail
{
    /////////////////////////////////////////////////////////////////////////
    // types -> type

    /// a...b -> a
    template
        < typename     U
        , typename ... T
        > struct first
    {
        using type = U;
    };

    /// a...b -> b
    template
        < typename    U
        , typename ...T
        > struct last
    {
        using type = typename last<T...>::type;
    };

    template
        < typename U
        > struct last<U>
    {
        using type = U;
    };

    /// f -> g a -> f a
    template
        < template<typename ...> class M
        , typename                     T
        > struct map
    {};

    template
        < template<typename ...> class F
        , template<typename ...> class T
        , typename                 ... V
        > struct map<F, T<V...>>
    {
        using type = F<V...>;
    };

    /// (a -> bool) -> a... -> bool
    template
        < template<typename   > class F
        , typename                ... T
        > struct exist_if
    {
        bool constexpr static value = (F<T>::value || ...);
    };

}}

namespace uncat { namespace detail
{
    /////////////////////////////////////////////////////////////////////////
    // types -> types

    /// a helper to pack types into a type.
    template
        < typename ...T
        > struct pack {};

    /// [a] -> [b] -> [a, b]
    template
        < typename L
        , typename R
        > struct join
    {};

    template
        < template<typename ...> class C
        , typename ...L
        , typename ...R
        > struct join<C<L...>, C<R...>>
    {
        using type = C<L..., R...>;
    };

    /// [a...b] -> [b...a]
    template
        < typename T
        > struct reverse
    {};

    template
        < template<typename ...> class T
        , typename                     U
        , typename                 ... V
        > struct reverse<T<U, V...>>
    {
        using type = typename join
            < typename reverse<T<V...>>::type
            , T<U>
            >::type;
    };

    template
        < template<typename ...> class T
        > struct reverse<T<>>
    {
        using type = T<>;
    };

    /// f -> [...] -> [..]
    template
        < template<typename> class F // filter functor
        , typename T                 // something like T<U...>
        > struct filter
    {};

    template
        < template<typename   > class F
        , template<typename...> class T
        , typename                    U
        , typename                ... V
        > struct filter<F, T<U, V...>>
    {
        using type = typename join
            < typename select<F<U>::value, T<U>, T<>>::type
            , typename filter<F, T<V...>>::type
            >::type;
    };

    template
        < template<typename   > class F
        , template<typename...> class T
        > struct filter<F, T<>>
    {
        using type = T<>;
    };

    /// (a -> bool) -> a... -> type / bool
    template
        < template<typename> class F // comparison
        , typename                 T // something like T<U...>
        , typename = void            // SFINAE
        > struct find_if
    {
        bool constexpr static value = false;
    };

    template
        < template<typename   > class F
        , template<typename...> class T
        , typename                    U
        , typename                ... V
        > struct find_if
        < F
        , T<U, V...>
        , std::enable_if_t<!F<U>::value && exist_if<F, V...>::value>
        >
    {
        bool constexpr static value = true;
        using type = typename find_if<F, T<V...>>::type;
    };

    template
        < template<typename   > class F
        , template<typename...> class T
        , typename                    U
        , typename                ... V
        > struct find_if<F, T<U, V...>, std::enable_if_t<F<U>::value>>
    {
        bool constexpr static value = true;
        using type = U;
    };

    /// check if L is a subset of R.
    template
        < typename L
        , typename R
        , typename = void
        > struct is_subset
    {
        bool constexpr static value = false;
    };

    template
        < template<typename ...> class C
        , typename ...L
        , typename ...R
        > struct is_subset
            < C<L...>
            , C<R...>
            , std::void_t<typename find_if
                < same<L>::template type
                , C<R...>
                >::type...>
            >
    {
        bool constexpr static value = true;
    };

    /// distinct
    template
        < typename T
        > struct distinct
    {};

    template
        < template<typename ...> class C
        , typename T
        , typename ...U
        > struct distinct<C<T, U...>>
    {
        using type = typename join
            < typename select
                < find_if<same<T>::template type, C<U...>>::value
                , C<>
                , C<T>
                >::type
            , typename distinct<C<U...>>::type
            >::type;
    };

    template
        < template<typename ...> class C
        > struct distinct<C<>>
    {
        using type = C<>;
    };

    /// distinct_stable
    template
        < typename T
        > struct distinct_stable
    {};

    template
        < template<typename ...> class C
        , typename                 ... T
        > struct distinct_stable<C<T...>>
    {
        using type = typename reverse
            < typename distinct
                < typename reverse<C<T...>>::type
                >::type
            >::type;
    };

}}

namespace uncat
{
    /////////////////////////////////////////////////////////////////////////
    // export

    struct types
    {
        template
            < template<typename ...> class M
            , typename                     T
            > using map_t = typename detail::map<M, T>::type;

        template
            < typename L
            , typename R
            > using join_t = typename detail::join<L, R>::type;

        template
            < bool     C
            , typename L
            , typename R
            > using select_t = typename detail::select<C, L, R>::type;

        template
            < typename ...T
            > using last_t = typename detail::last<T...>::type;

        template
            < typename ...T
            > using first_t = typename detail::first<T...>::type;

        template
            < template<typename> class F
            , typename             ... T
            > using find_if = detail::find_if<F, detail::pack<T...>>;

        template
            < typename    T
            , typename ...U
            > using find = find_if
                < detail::same<T>::template type
                , U...
                >;

        template
            < typename    T
            , typename ...U
            > using find_t = typename find<T, U...>::type;

        template
            < typename    T
            , typename ...U
            > bool constexpr static find_v = find<T, U...>::value;

        template
            < typename T
            > using remove_cvr_t = detail::remove_cvr_t<T>;
    };
}
