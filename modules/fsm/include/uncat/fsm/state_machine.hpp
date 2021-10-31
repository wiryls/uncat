#pragma once
#include <variant>
#include <uncat/types/types.hpp>

namespace uncat { namespace fsm
{
    template
        < typename F
        , typename T >
    struct validator
        : std::false_type
    {};

    template
        < typename                     F
        , template<typename ...> class T
        , typename                     S
        , typename                     D
        , typename                 ... I >
    struct validator
        < F
        , T<S, D, I ...> >
        : std::bool_constant<(std::is_invocable_r_v<D, F, std::add_lvalue_reference_t<S>, I> && ...)>
    {};

    template
        < template<typename ...> class O
        , typename     M
        , typename ... T
        > struct parser
    {
        using  action = M;
        using  states = O<>;
        using  inputs = O<>;
        using matches = O<>;
    };

    template
        < template<typename ...> class O
        , typename                     M
        , template<typename ...> class T
        , typename                     S
        , typename                     D
        , typename                 ... I
        , typename                 ... U >
    requires validator<M, T<S, D, I...>>::value && (validator<M, U>::value && ...)
    struct parser
        < O
        , M
        , T<S, D, I...>
        , U ...
        >
    {
        using  action = M;
        using  states = O<>;
        using  inputs = O<>;
        using matches = O<>;
    };

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
        using states = typename types::distinct_stable_t
            < typename types::join
                < V<S1, S2>
                , typename transition_parser<F, V<U...>>::states
                > ::type
            >;
        using inputs = typename types::distinct_stable_t
            < typename types::join
                < V<I ...>
                , typename transition_parser<F, V<U...>>::inputs
                > ::type
            >;
        using matches = types::join_t
            < V<V<S1, I>...>
            , typename transition_parser<F, V<U...>>::matches
            >;
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
        template<typename I> using input_t = types::map_t<types::find_t, types::join_t<types::pack<I>, inputs>>;
        template<typename I> using state_t = types::map_t<types::find_t, types::join_t<types::pack<I>, states>>;

    public:
        template<typename I> using  bool_t = types::first_t<bool, input_t<I>>;

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
        using holder = types::map_t<std::variant, states>;
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
        : state(types::map_t<types::first_t, states>())
        , shift(F())
    {}

    template<typename F, typename ...T>
    template<typename I> inline
    typename state_machine<F, T...>::template bool_t<I> state_machine<F, T...>::
    accept(I && input)
    {
        return std::visit([this, &input](auto & current) -> bool
        {
            using state_t = std::remove_cvref_t<decltype(current)>;
            using input_t = std::remove_cvref_t<I>;
            using match_t = typename parser::matches;

            using types::pack;
            auto constexpr  acceptable = types::map_t<types::find, types::join_t<pack<pack<state_t, input_t>>, match_t>>::value;
            if   constexpr (acceptable)
                state = shift(current, std::forward<I>(input));
            return acceptable;
        }, state);
    }
}
