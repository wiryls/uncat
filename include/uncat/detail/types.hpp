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
        > struct select_type
    {
        using type = R;
    };

    template
        < typename L
        , typename R
        > struct select_type<true, L, R>
    {
        using type = L;
    };

    /// same_to_type partially fills a std::is_same with T.
    template
        < typename T
        > struct same_to_type
    {
        template<typename U>
        using type = std::is_same<T, U>;
    };

    template
        < template<typename> class T
        > struct not_value_type
    {
        template
            < typename U
            > struct type
        {
            auto constexpr static value = !T<U>::value;
        };
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
        > struct last_type_base
    {};

    template
        < template<typename ...> class C
        , typename ...T
        , typename    U
        > struct last_type_base<C, C<U, T...>>
    {
        using type = typename last_type_base<C, C<T...>>::type;
    };

    template
        < template<typename ...> class C
        , typename T
        > struct last_type_base<C, C<T>>
    {
        using type = T;
    };

    /// find a type T by M<T>::value
    template
        < template<typename ...> class C // type containter
        , template<typename>     class M // comparison
        , typename T                     // something like C<U...>
        , typename = void                // SFINAE
        > struct find_if_base
    {
        bool constexpr static value = false;
    };

    template
        < template<typename ...> class C
        , template<typename>     class M
        , typename ...T
        , typename U
        > struct find_if_base
            < C
            , M
            , C<U, T...>
            , std::enable_if_t
                < !M<U>::value
                , std::void_t<typename find_if_base<C, M, C<T...>>::type>
                >
            >
    {
        using type = typename find_if_base<C, M, C<T...>>::type;
        bool constexpr static value = true;
    };

    template
        < template<typename ...> class C
        , template<typename>     class M
        , typename ...T
        , typename U
        > struct find_if_base
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
        > struct filter_base
    {};

    template
        < template<typename ...> class C
        , template<typename>     class M
        , typename ...T
        , typename ...U
        , typename V
        > struct filter_base<C, M, C<T...>, C<V, U...>>
    {
        using type = typename filter_base
            < C
            , M
            , typename select_type
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
        > struct filter_base<C, M, C<T...>, C<>>
    {
        using type = C<T...>;
    };

    /// check if L is a subset of R.
    template
        < template<typename ...> class C
        , typename L
        , typename R
        , typename = void
        > struct is_subset_base
    {
        bool constexpr static value = false;
    };

    template
        < template<typename ...> class C
        , typename ...L
        , typename ...R
        > struct is_subset_base
            < C
            , C<L...>
            , C<R...>
            , std::void_t<typename find_if_base
                < C
                , same_to_type<L>::template type
                , C<R...>
                >::type...>
            >
    {
        bool constexpr static value = true;
    };

}}
