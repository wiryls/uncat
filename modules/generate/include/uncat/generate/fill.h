#pragma once
#include <concepts>
#include <iterator>
#include <random>
#include <ranges>
#include <type_traits>

namespace uncat::generate {

template <std::forward_iterator O, std::random_access_iterator I>
inline auto fill_with(O first, O last, I input_first, I input_last) -> void
requires requires(O o, I i) { *o = *i; }
{
    using difference_type = typename std::iterator_traits<I>::difference_type;

    if (first != last && input_first != input_last)
    {
        auto thread_local static engine = std::mt19937{std::random_device{}()};

        auto limit = std::max<difference_type>(std::distance(input_first, input_last), 1);
        auto dist  = std::uniform_int_distribution<difference_type>(0, limit - 1);

        for (; first != last; ++first)
            *first = *(input_first + dist(engine));
    }
}

template <std::ranges::forward_range O, std::ranges::random_access_range I>
inline auto fill_with(O & output, I const & input) -> void
requires std::same_as<std::ranges::range_value_t<I>, std::ranges::range_value_t<O>>
{
    auto output_first = std::ranges::begin(output);
    auto output_last  = std::ranges::end(output);
    auto input_first  = std::ranges::begin(input);
    auto input_last   = std::ranges::end(input);

    using value_type = std::ranges::range_value_t<I>;
    if constexpr (std::ranges::contiguous_range<I> && std::is_array_v<I> && std::same_as<value_type, typename std::char_traits<value_type>::char_type>)
        --input_last;

    fill_with(output_first, output_last, input_first, input_last);
}

} // namespace uncat::generate
