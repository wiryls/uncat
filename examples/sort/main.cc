#include "quick_sort.h"
#include <iostream>

template <typename I, I... xs> void output(std::integer_sequence<I, xs...>)
{
    (std::cout << ... << xs) << std::endl;
}

template <typename I, I x, I y, I... xs> void output(std::integer_sequence<I, x, y, xs...>)
{
    std::cout << x << ',';
    output(std::integer_sequence<I, y, xs...>());
}

int main()
{
    output(quicksort_t<int, 2, 1, 4, 2, 3, 4>());
    return 0;
}
