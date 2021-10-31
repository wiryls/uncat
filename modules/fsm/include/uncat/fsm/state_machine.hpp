#pragma once
#include <variant>
#include <uncat/types/types.hpp>

namespace uncat { namespace fsm
{
    template
        < typename F
        , typename V
        , typename = void
        > struct transition_parser {};

    template
        < typename F
        , template<typename ...> class V
        , template<typename ...> class T
        , typename    S1
        , typename    S2
        , typename ...I
        , typename ...U
        > struct transition_parser
        < F
        , V<T<S1, S2, I...>, U...>
        , std::void_t
            < typename types::functor_pass<F, S2, S1&, I>::type ...
            , typename transition_parser<F, V<U...>>::states
            , typename transition_parser<F, V<U...>>::inputs
            >
        >
    {
        using action = F;
        using states = typename types::distinct_stable
            < typename types::join
                < V<S1, S2>
                , typename transition_parser<F, V<U...>>::states
                > ::type
            > ::type;
        using inputs = typename types::distinct_stable
            < typename types::join
                < V<I ...>
                , typename transition_parser<F, V<U...>>::inputs
                > ::type
            > ::type;
        using matches = typename types::join
            < V<V<S1, I>...>
            , typename transition_parser<F, V<U...>>::matches
            > ::type;
    };

    template
        < typename F
        , template<typename ...> class V
        > struct transition_parser<F, V<>>
    {
        using  action = F;
        using  states = V<>;
        using  inputs = V<>;
        using matches = V<>;
    };
}}

namespace uncat
{
    template
        < typename S1
        , typename S2
        , typename ...T
        > struct transition {};

    template
        < typename    F
        , typename ...T
        > struct state_machine
        : private types::pack<typename fsm::transition_parser<F, types::pack<T...>>::action>
        // if an error happens at action, it means `F` may not support some state-input pairs.
    {
    private:
        using parser = fsm::transition_parser<F, types::pack<T...>>;
        using states = typename parser::states;
        using inputs = typename parser::inputs;
        template<typename I> using input_t = map_t<find_t, join_t<types::pack<I>, inputs>>;
        template<typename I> using state_t = map_t<find_t, join_t<types::pack<I>, states>>;

    public:
        template<typename I> using  bool_t = first_t<bool, input_t<I>>;

    public:
        template<typename S, typename X, typename = std::void_t<state_t<S>>>
        state_machine(S && state, X && action);
        state_machine();

        state_machine            (state_machine const &) = default;
        state_machine            (state_machine      &&) = default;
        state_machine & operator=(state_machine const &) = default;
        state_machine & operator=(state_machine      &&) = default;

    public:
        template<typename I> bool_t<I> accept(I && input);

    private:
        using holder = map_t<std::variant, states>;
        holder state;
        F      shift;
    };

    template<typename F, typename ...T>
    template<typename S, typename X, typename> inline
    state_machine<F, T...>::
    state_machine(S && init, X && next)
        : state(std::forward<S>(init))
        , shift(std::forward<X>(next))
    {}

    template<typename F, typename ...T> inline
    state_machine<F, T...>::
    state_machine()
        : state(map_t<first_t, states>())
        , shift(F())
    {}

    template<typename F, typename ...T>
    template<typename I> inline
    typename state_machine<F, T...>::template bool_t<I> state_machine<F, T...>::
    accept(I && input)
    {
        return std::visit([this, &input](auto & current) -> bool
        {
            using state_t = remove_cvr_t<decltype(current)>;
            using input_t = remove_cvr_t<I>;
            using match_t = typename parser::matches;

            using types::pack;
            auto constexpr  acceptable = map_t<find, join_t<pack<pack<state_t, input_t>>, match_t>>::value;
            if   constexpr (acceptable)
                state = shift(current, std::forward<I>(input));
            return acceptable;
        }, state);
    }
}
