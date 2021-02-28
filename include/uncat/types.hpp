#pragma once
#include <uncat/detail/types.hpp>

namespace uncat
{
    template
        < typename ...T
        > struct last_type : detail::last_type_base
            < detail::pack
            , detail::pack<T...>
            >
    {};

    template
        < template<typename> class M
        , typename ...T
        > struct find_if : detail::find_if_base
            < detail::pack
            , M
            , detail::pack<T...>
            >
    {};

    template
        < typename    T
        , typename ...U
        > struct find : find_if
            < detail::same_to_type<T>::template type
            , U...
            >
    {};
}
