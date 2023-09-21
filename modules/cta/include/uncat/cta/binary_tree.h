#pragma once

#include <uncat/cta/common.h>

namespace uncat { namespace binary_tree {

template <typename ValueType, ValueType value, typename LeftChild = void, typename RightChild = void> struct node
{};

}} // namespace uncat::binary_tree

namespace uncat { namespace binary_tree { namespace aux {

namespace aux = cta::aux;

enum struct order
{
    pre,
    in,
    post,
};

template <typename I> struct pre_in_post_order
{
private:
    template <I... xs> using list = aux::list<I, xs...>;

public:
    template <order, typename N> struct traversal
    {
        using type = list<>;
    };

    template <order o, typename N> using traversal_t = typename traversal<o, N>::type;

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
};

}}} // namespace uncat::binary_tree::aux

namespace uncat { namespace binary_tree {

template <typename T> struct pre_order
{};

template <typename I, I x, typename L, typename R> struct pre_order<node<I, x, L, R>>
{
    using type = aux::pre_in_post_order<I>::template traversal_t<aux::order::pre, node<I, x, L, R>>;
};

template <typename T> struct in_order
{};

template <typename I, I x, typename L, typename R> struct in_order<node<I, x, L, R>>
{
    using type = aux::pre_in_post_order<I>::template traversal_t<aux::order::in, node<I, x, L, R>>;
};

template <typename T> struct post_order
{};

template <typename I, I x, typename L, typename R> struct post_order<node<I, x, L, R>>
{
    using type = aux::pre_in_post_order<I>::template traversal_t<aux::order::post, node<I, x, L, R>>;
};

template <typename N> struct level_order
{};

template <typename I, I x, typename L, typename R> struct level_order<node<I, x, L, R>>
{
private:
    template <I... xs> using list = cta::aux::list<I, xs...>;

    template <typename... N> struct traversal
    {
        using type = list<>;
    };

    template <typename... N> struct traversal<void, N...>
    {
        using type = typename traversal<N...>::type;
    };

    template <typename... N, I value, typename L, typename R> struct traversal<node<I, value, L, R>, N...>
    {
        using type = typename traversal<N..., L, R>::type::template push_front<value>;
    };

public:
    using type = typename traversal<node<I, x, L, R>>::type::to_integer_sequence;
};

}} // namespace uncat::binary_tree

namespace uncat {

template <typename I, I value, typename Left = void, typename Right = void>
using binary_tree_node = typename binary_tree::node<I, value, Left, Right>;

template <typename Node> using pre_order_traversal_t   = typename binary_tree::post_order<Node>::type;
template <typename Node> using in_order_traversal_t    = typename binary_tree::in_order<Node>::type;
template <typename Node> using post_order_traversal_t  = typename binary_tree::pre_order<Node>::type;
template <typename Node> using level_order_traversal_t = typename binary_tree::level_order<Node>::type;

} // namespace uncat
