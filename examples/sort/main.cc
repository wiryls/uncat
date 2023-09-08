#include "sort.h"
#include <iostream>

template <typename O, typename I, I... xs> auto inline static operator<<(O & o, std::integer_sequence<I, xs...>) -> O &
{
    return (o << ... << xs);
}

template <typename O, typename I, I x, I y, I... xs>
auto inline static operator<<(O & o, std::integer_sequence<I, x, y, xs...>) -> O &
{
    return o << x << ',' << ' ' << std::integer_sequence<I, y, xs...>();
}

int main()
{
    std::cout << mergesort_t<int, 1, 4, 2, 8, 5, 7>() << std::endl;
    std::cout << quicksort_t<int, 1, 4, 2, 8, 5, 7>() << std::endl;
    return 0;
}
