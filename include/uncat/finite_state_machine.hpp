#pragma once
#include <variant>
#include <uncat/detail/types.hpp>

namespace uncat { namespace detail
{
    template
        < template<typename, typename> class F
        , template<typename ...      > class T
        , typename ...V
        > struct transitions;

    template
        < template<typename, typename> class F
        , template<typename ...      > class T
        , typename S1
        , typename S2
        , typename ...I
        , typename ...V
        > struct transitions<F, T, T<S1, S2, I...>, V...>
        : functor_validator<F<S1, I>, S2, S1&, I>::type ...
        , transitions<F, T, V...>
    {};

    template
        < template<typename, typename> class F
        , template<typename ...      > class T
        > struct transitions<F, T>
    {};

    template
        < template<typename ...> class C
        , template<typename ...> class T
        , typename V
        > struct transitions_parser_base;

    template
        < template<typename ...> class C
        , template<typename ...> class T
        , typename S1
        , typename S2
        , typename ...I
        , typename ...V
        > struct transitions_parser_base<C, T, C<T<S1, S2, I...>, V...>>
    {
        using states = typename join
            < C
            , C<S1, S2>
            , typename transitions_parser_base<C, T, C<V...>>::states
            >::type;

        using triggers = typename join
            < C
            , C<I...>
            , typename transitions_parser_base<C, T, C<V...>>::triggers
            >::type;
    };

    template
        < template<typename ...> class C
        , template<typename ...> class T
        > struct transitions_parser_base<C, T, C<>>
    {
        using states   = C<>;
        using triggers = C<>;
    };

    template
        < template<typename ...> class C
        , template<typename ...> class T
        , typename                 ... V
        > struct transitions_parser
    {
        using states = typename distinct_stable
            < C
            , typename transitions_parser_base<C, T, C<V...>>::states
            >::type;

        using triggers = typename distinct_stable
            < C
            , typename transitions_parser_base<C, T, C<V...>>::triggers
            >::type;
    };

}}

namespace uncat
{
    template
        < typename S1
        , typename S2
        , typename ...I
        > struct transition {};

    template
        < template<typename, typename> class F
        , typename                       ... T
        > struct state_machine
        : types
        , detail::transitions<F, transition, T...>
    {
    private:
        template<typename ...V> struct list {};
        using parser = detail::transitions_parser<list, transition, T...>;

    public:
        using   states = typename parser::states;
        using triggers = typename parser::triggers;
        template<typename U> using trigger_t =   map_t<join_t<list, list<U>, triggers>, find_t>;
        template<typename U> using    bool_t = first_t<bool, trigger_t<U>>;

    public:
        template<typename U> bool_t<U> accept(U && trigger);

    private:
        using holder = map_t<states, std::variant>;
        holder state = map_t<states, first_t>();
    };

    template<template<typename, typename> class F, typename ...T>
    template<typename U> inline
    typename state_machine<F, T...>::template bool_t<U> state_machine<F, T...>::
    accept(U && trigger)
    {
        return std::visit([this, &trigger](auto & current) -> bool
        {
            using   state_t = remove_cvr_t<decltype(current)>;
            using trigger_t = remove_cvr_t<U>;
            using    this_t = decltype(this);
            using    that_t = F<state_t, trigger_t>*;

            auto constexpr  acceptable = std::is_convertible_v<this_t, that_t>;
            if   constexpr (acceptable)
            {
                auto & next = *static_cast<that_t>(this);
                this->state = next(current, std::forward<U>(trigger));
            }

            return acceptable;
        }, state);
    }
}
