#pragma once

#include <uncat/cta/common.h>

namespace uncat::binary_tree {

template <typename ValueType, ValueType value, typename LeftChild = void, typename RightChild = void> struct node
{};

} // namespace uncat::binary_tree

namespace uncat::binary_tree::aux {

namespace aux = cta::aux;

enum struct order
{
    pre,
    in,
    post,
    level,
};

template <typename I> struct any_order
{
private:
    template <I... xs> using list = aux::list<I, xs...>;

public:
    template <order, typename... N> struct traversal
    {
        using type = list<>;
    };

    template <order o, typename... N> using traversal_t = typename traversal<o, N...>::type;

    template <I x, typename L, typename R> struct traversal<order::pre, node<I, x, L, R>>
    {
        using type = aux::join_t<list<x>, traversal_t<order::pre, L>, traversal_t<order::pre, R>>;
    };

    template <I x, typename L, typename R> struct traversal<order::in, node<I, x, L, R>>
    {
        using type = aux::join_t<traversal_t<order::in, L>, list<x>, traversal_t<order::in, R>>;
    };

    template <I x, typename L, typename R> struct traversal<order::post, node<I, x, L, R>>
    {
        using type = aux::join_t<traversal_t<order::post, L>, traversal_t<order::post, R>, list<x>>;
    };

    template <typename... N> struct traversal<order::level, void, N...>
    {
        using type = traversal_t<order::level, N...>;
    };

    template <typename... N, I value, typename L, typename R> struct traversal<order::level, node<I, value, L, R>, N...>
    {
        using type = traversal_t<order::level, N..., L, R>::template push_front<value>;
    };
};

} // namespace uncat::binary_tree::aux

namespace uncat::binary_tree {

// Export
template <typename T> struct pre_order
{};

template <typename I, I x, typename L, typename R> struct pre_order<node<I, x, L, R>>
{
    using type = aux::any_order<I>::template traversal_t<aux::order::pre, node<I, x, L, R>>;
};

template <typename T> struct in_order
{};

template <typename I, I x, typename L, typename R> struct in_order<node<I, x, L, R>>
{
    using type = aux::any_order<I>::template traversal_t<aux::order::in, node<I, x, L, R>>;
};

template <typename T> struct post_order
{};

template <typename I, I x, typename L, typename R> struct post_order<node<I, x, L, R>>
{
    using type = aux::any_order<I>::template traversal_t<aux::order::post, node<I, x, L, R>>;
};

template <typename N> struct level_order
{};

template <typename I, I x, typename L, typename R> struct level_order<node<I, x, L, R>>
{
    using type = aux::any_order<I>::template traversal_t<aux::order::level, node<I, x, L, R>>;
};

} // namespace uncat::binary_tree

namespace uncat {

template <typename I, I value, typename Left = void, typename Right = void>
using binary_tree_node = typename binary_tree::node<I, value, Left, Right>;

template <typename Node> using pre_order_traversal_t   = typename binary_tree::pre_order<Node>::type;
template <typename Node> using in_order_traversal_t    = typename binary_tree::in_order<Node>::type;
template <typename Node> using post_order_traversal_t  = typename binary_tree::post_order<Node>::type;
template <typename Node> using level_order_traversal_t = typename binary_tree::level_order<Node>::type;

} // namespace uncat
