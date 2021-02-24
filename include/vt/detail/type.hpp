#pragma once
#include <type_traits>

namespace vt { namespace detail
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

    /// last_type returns the last type
    template
        < typename T
        , typename ... U
        > struct last_type
    {
        using type = typename last_type<U...>::type;
    };

    template
        < typename T
        > struct last_type<T>
    {
        using type = T;
    };
}}
