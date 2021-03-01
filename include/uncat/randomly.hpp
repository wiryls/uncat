#pragma once

#include <type_traits>
#include <iterator>
#include <random>
#include <string>

namespace uncat
{
    template
        < typename O
        , typename I
        > inline std::enable_if_t
            <  std::is_same_v
                < typename std::iterator_traits<I>::iterator_category
                , std::random_access_iterator_tag
                >
            && std::is_same_v
                < typename std::iterator_traits<I>::value_type
                , typename std::iterator_traits<O>::value_type
                >
            >
    fill_random(O first, O last, I input_first, I input_last)
    {
        if (first != last && input_first != input_last)
        {
            using difference_type = typename std::iterator_traits<I>::difference_type;
            auto thread_local static engine = std::mt19937{ std::random_device{}() };
            auto n = std::max(std::distance(input_first, input_last), static_cast<difference_type>(1));
            auto d = std::uniform_int_distribution<difference_type>(0, n - 1);

            for (; first != last; ++first)
                *first = *(input_first + d(engine));
        }
    }

    template
        < typename O
        , typename C
        , std::size_t N
        > inline std::enable_if_t
            < std::is_same_v
                < typename std::    char_traits<C>::char_type
                , typename std::iterator_traits<O>::value_type
                >
            >
    fill_random(O first, O last, C const (&input)[N])
    {
        // we should ignore the last '\0' if it is a string literal.
        fill_random(first, last, std::begin(input), std::next(std::end(input), -1));
    }
}
