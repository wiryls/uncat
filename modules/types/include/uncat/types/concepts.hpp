#pragma once
#include <uncat/types/types.hpp>

namespace uncat { namespace types
{
    template
        < typename     H
        , typename ... T >
    concept oneof = exists_v<H, T...>;

    template
        < typename T
        , typename C >
    concept in = is_in_v<T, C>;
}}
