#pragma once
#include <type_traits>
#include "type.hpp"

namespace orz { namespace detail
{
    /////////////////////////////////////////////////////////////////////////
    // operations on a type list

    /// find a type T by M<T>::value
    template
        < template<typename ...> class C // type containter
        , template<typename>     class M // comparison
        , typename T                     // is C<U...>
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
            , std::void_t<typename find_if_base<C, M, C<T...>>::type>
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
    
    template
        < template<typename> class M
        , typename ...T
        > struct find_if : find_if_base<pack, M, pack<T...>>
    {};

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
