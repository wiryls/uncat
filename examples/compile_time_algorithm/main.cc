#include <iostream>
#include <utility>

#include <uncat/cta/binary_tree.h>
#include <uncat/cta/common.h>
#include <uncat/cta/sort.h>

int main()
{
    using uncat::operator<<;

    // NOLINT(cppcoreguidlines-avoid-magic-numbers)
    using list = std::integer_sequence<int, 1, -2, 3, -4, 5, -6>;
    using out1 = uncat::merge_sort_t<list>;
    std::cout << out1() << std::endl;

    using out2 = uncat::quick_sort_t<out1, uncat::greater>;
    std::cout << out2() << std::endl;

    using uncat::binary_tree::node;
    // NOLINT(cppcoreguidlines-avoid-magic-numbers)
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
              << uncat::level_order_traversal_t<tree>() << std::endl;
    return 0;
}
