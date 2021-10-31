#pragma once
#include <type_traits>

namespace uncat { namespace types
{
    /////////////////////////////////////////////////////////////////////////
    // limited types -> type / value

    /// remove const reference of a type
    template
        < typename T
        > using remove_cvr_t = std::remove_cv_t<std::remove_reference_t<T>>;

    /// curry
    template
        < template<typename ...> class T
        , typename                 ... U
        > struct curry
    {
        template
            < typename ... V
            > using type = T<U..., V...>;
    };

    /// make a non variadic template from a variadic template
    template
        < template<typename ...> class T
        , std::size_t                  N = 0
        > struct non_variadic
    {
        template
            < typename ... U
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

    template
        < template<typename ...> class T
        > struct non_variadic<T, 2>
    {
        template
            < typename V1, typename V2
            > using type = typename T<V1, V2>::type;
    };

    /// same partially fills a std::is_same with T.
    template
        < typename T
        > using same = non_variadic<curry<std::is_same, T>::template type, 1>;

    /// make sure the functor F could be invoked like F()(T()...) -> R
    template
        < typename     F
        , typename     R
        , typename ... T
        > struct functor_pass
    {
        bool constexpr static value = false;
    };

    template
        < typename     F
        , typename ... T
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

namespace uncat { namespace types
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
        < typename     U
        , typename ... T
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
        < typename     T
        , typename ... V
        > struct exists : std::bool_constant<(std::is_same_v<T, V> || ...)>
    {};

    /// (a -> bool) -> a... -> bool
    template
        < template<typename> class F
        , typename             ... T
        > struct exists_if
    {
        bool constexpr static value = (F<T>::value || ...);
    };

}}

namespace uncat { namespace types
{
    /////////////////////////////////////////////////////////////////////////
    // types -> types

    /// a helper to pack types into a type.
    template
        < typename ... T
        > struct pack {};

    /// [a] -> [b] -> [a, b]
    template
        < typename L
        , typename R
        > struct join
    {};

    template
        < template<typename ...> class C
        , typename                 ... L
        , typename                 ... R
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
        < template<typename> class F // filter
        , typename                 T // something like T<U...>
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
            < std::conditional_t
                < F<U>::value
                , T<U>
                , T<>
                >
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
        , std::enable_if_t<!F<U>::value && exists_if<F, V...>::value>
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
        > struct is_subset
    {
        bool constexpr static value = false;
    };

    template
        < template<typename ...> class C
        , typename                 ... L
        , typename                 ... R
        > struct is_subset
            < C<L...>
            , C<R...>
            >
    {
        bool constexpr static value = (exists<L, R...>::value && ...);
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
            < std::conditional_t
                < exists<T, U...>::value
                , C<>
                , C<T>
                >
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

    template
        < template<typename ...> class M
        , typename                     T
        > using map_t = typename types::map<M, T>::type;

    template
        < typename L
        , typename R
        > using join_t = typename types::join<L, R>::type;

    template
        < typename ... T
        > using last_t = typename types::last<T...>::type;

    template
        < typename ... T
        > using first_t = typename types::first<T...>::type;

    template
        < template<typename> class F
        , typename             ... T
        > using find_if = types::find_if<F, types::pack<T...>>;

    template
        < typename     T
        , typename ... U
        > using find = find_if
            < types::same<T>::template type
            , U...
            >;

    template
        < typename     T
        , typename ... U
        > using find_t = typename find<T, U...>::type;

    template
        < typename     T
        , typename ... U
        > bool constexpr static find_v = find<T, U...>::value;

    template
        < typename T
        > using remove_cvr_t = types::remove_cvr_t<T>;
}
