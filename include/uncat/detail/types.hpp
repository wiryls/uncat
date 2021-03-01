#pragma once
#include <type_traits>

namespace uncat { namespace detail
{
    /////////////////////////////////////////////////////////////////////////
    // operations on limited number of types

    /// a helper to pack types into a type.
    template
        < typename ...T
        > struct pack {};

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

    /// same_to_type partially fills a std::is_same with T.
    template
        < typename T
        > struct same_to
    {
        template<typename U>
        using type = std::is_same<T, U>;
    };
}}

namespace uncat { namespace detail
{
    /////////////////////////////////////////////////////////////////////////
    // operations on a type list

    /// last type in list
    template
        < template<typename ...> class C
        , typename T
        > struct last
    {};

    template
        < template<typename ...> class C
        , typename ...T
        , typename    U
        > struct last<C, C<U, T...>>
    {
        using type = typename last<C, C<T...>>::type;
    };

    template
        < template<typename ...> class C
        , typename T
        > struct last<C, C<T>>
    {
        using type = T;
    };

    /// find a type T by M<T>::value
    template
        < template<typename ...> class C // type containter
        , template<typename>     class M // comparison
        , typename T                     // something like C<U...>
        , typename = void                // SFINAE
        > struct find_if
    {
        bool constexpr static value = false;
    };

    template
        < template<typename ...> class C
        , template<typename>     class M
        , typename ...T
        , typename U
        > struct find_if
            < C
            , M
            , C<U, T...>
            , std::enable_if_t
                < !M<U>::value
                , std::void_t<typename find_if<C, M, C<T...>>::type>
                >
            >
    {
        using type = typename find_if<C, M, C<T...>>::type;
        bool constexpr static value = true;
    };

    template
        < template<typename ...> class C
        , template<typename>     class M
        , typename ...T
        , typename U
        > struct find_if
            < C
            , M
            , C<U, T...>
            , std::enable_if_t<M<U>::value>
            >
    {
        using type = U;
        bool constexpr static value = true;
    };


    /// filter a type list to pack<...>
    template
        < template<typename ...> class C
        , template<typename>     class M
        , typename T
        , typename U
        > struct filter
    {};

    template
        < template<typename ...> class C
        , template<typename>     class M
        , typename ...T
        , typename ...U
        , typename V
        > struct filter<C, M, C<T...>, C<V, U...>>
    {
        using type = typename filter
            < C
            , M
            , typename select
                < M<V>::value
                , C<T..., V>
                , C<T...>
                >::type
            , C<U...>
            >::type;
    };

    template
        < template<typename ...> class C
        , template<typename>     class M
        , typename ...T
        > struct filter<C, M, C<T...>, C<>>
    {
        using type = C<T...>;
    };

    /// check if L is a subset of R.
    template
        < template<typename ...> class C
        , typename L
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
            < C
            , C<L...>
            , C<R...>
            , std::void_t<typename find_if
                < C
                , same_to<L>::template type
                , C<R...>
                >::type...>
            >
    {
        bool constexpr static value = true;
    };

}}

namespace uncat
{
    /////////////////////////////////////////////////////////////////////////
    // export

    struct types
    {
        template
            < bool     C
            , typename L
            , typename R
            > using select_t = typename detail::select<C, L, R>::type;

        template
            < typename ...T
            > struct last : detail::last
            < detail::pack
            , detail::pack<T...>
            >
        {};

        template
            < typename ...T
            > using last_t = typename last<T...>::type;

        template
            < template<typename> class M
            , typename ...T
            > struct find_if : detail::find_if
            < detail::pack
            , M
            , detail::pack<T...>
            >
        {};

        template
            < typename    T
            , typename ...U
            > struct find : find_if
            < detail::same_to<T>::template type
            , U...
            >
        {};

        template
            < typename    T
            , typename ...U
            > using find_t = typename find<T, U...>::type;
    };
}
