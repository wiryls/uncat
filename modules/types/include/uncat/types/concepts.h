#pragma once
#include <uncat/types/types.h>

namespace uncat { namespace types {

template <typename H, typename... T>
concept oneof = exists_v<H, T...>;

template <typename T, typename C>
concept in = is_in_v<T, C>;

}} // namespace uncat::types
