#include <iostream>
#include <utility>

#include <uncat/cta/binary_tree.h>
#include <uncat/cta/common.h>
#include <uncat/cta/sort.h>

// NOLINTNEXTLINE(bugprone-exception-escape)
int main()
{
    using uncat::cta::operator<<;
    {
        using list = std::integer_sequence<int, 1, -2, 3, -4, 5, -6>;
        using out1 = uncat::merge_sort_t<list>;
        std::cout << out1() << '\n';

        using out2 = uncat::quick_sort_t<list, uncat::cta::greater>;
        std::cout << out2() << '\n';
    }
    {
        using uncat::cta::binary_tree::node;
        using tree = node<int, 0, node<int, 1, node<int, 3>>, node<int, 2, void, node<int, 4, node<int, 5>>>>;
        /*     0
         *    / \
         *   1   2
         *  /     \
         * 3       4
         *        /
         *       5
         */

        std::cout << uncat::pre_order_traversal_t<tree>() << '\n'
                  << uncat::in_order_traversal_t<tree>() << '\n'
                  << uncat::post_order_traversal_t<tree>() << '\n'
                  << uncat::level_order_traversal_t<tree>() << '\n';
    }
    return 0;
}
