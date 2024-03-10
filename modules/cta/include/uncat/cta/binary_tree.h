#pragma once
#include <uncat/cta/common.h>

namespace uncat::cta::binary_tree
{

template <auto value, typename Left = void, typename Right = void> struct node
{};

enum struct order
{
    pre,
    in,
    post,
    level,
};

} // namespace uncat::cta::binary_tree

namespace uncat::cta::binary_tree::aux
{

template <order o, typename... N> struct traversal;
template <order o, typename... N> using traversal_t = typename traversal<o, N...>::type;

template <order, typename... N> struct traversal
{
    using type = list<>;
};

template <auto x, typename L, typename R> struct traversal<order::pre, node<x, L, R>>
{
    using type = join_t<list<x>, traversal_t<order::pre, L>, traversal_t<order::pre, R>>;
};

template <auto x, typename L, typename R> struct traversal<order::in, node<x, L, R>>
{
    using type = join_t<traversal_t<order::in, L>, list<x>, traversal_t<order::in, R>>;
};

template <auto x, typename L, typename R> struct traversal<order::post, node<x, L, R>>
{
    using type = join_t<traversal_t<order::post, L>, traversal_t<order::post, R>, list<x>>;
};

template <typename... N> struct traversal<order::level, void, N...>
{
    using type = traversal_t<order::level, N...>;
};

template <typename... N, auto value, typename L, typename R> struct traversal<order::level, node<value, L, R>, N...>
{
    using type = typename traversal_t<order::level, N..., L, R>::template push_front<value>;
};

} // namespace uncat::cta::binary_tree::aux

namespace uncat::cta::binary_tree
{

template <order o, typename N> struct any_order
{};

template <order o, auto x, typename L, typename R> struct any_order<o, node<x, L, R>>
{
    using type = aux::traversal_t<o, node<x, L, R>>;
};

} // namespace uncat::cta::binary_tree

namespace uncat
{

template <auto value, typename Left = void, typename Right = void>
using binary_tree_node = cta::binary_tree::node<value, Left, Right>;

template <typename Node>
using pre_order_traversal_t = typename cta::binary_tree::any_order<cta::binary_tree::order::pre, Node>::type;
template <typename Node>
using in_order_traversal_t = typename cta::binary_tree::any_order<cta::binary_tree::order::in, Node>::type;
template <typename Node>
using post_order_traversal_t = typename cta::binary_tree::any_order<cta::binary_tree::order::post, Node>::type;
template <typename Node>
using level_order_traversal_t = typename cta::binary_tree::any_order<cta::binary_tree::order::level, Node>::type;

} // namespace uncat
