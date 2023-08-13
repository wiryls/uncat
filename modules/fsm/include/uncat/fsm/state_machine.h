#pragma once
#include <variant>

#include <uncat/types/concepts.h>
#include <uncat/types/types.h>

namespace uncat { namespace fsm { namespace aux {

template <typename F, typename T> struct validator : std::false_type
{};

template <typename F, template <typename...> class T, typename S, typename D, typename... I>
struct validator<F, T<S, D, I...>>
    : std::bool_constant<(std::is_invocable_r_v<D, F, std::add_lvalue_reference_t<S>, I> && ...)>
{};

template <template <typename...> class O, typename M, typename... T> struct parser
{
private:
    template <typename... U> struct collect
    {
        using action  = M;
        using states  = O<>;
        using inputs  = O<>;
        using matches = O<>;
    };

    template <template <typename...> class C, typename S, typename D, typename... I, typename... U>
        requires validator<M, C<S, D, I...>>::value && (validator<M, U>::value && ...)
    struct collect<C<S, D, I...>, U...>
    {
        using action  = M;
        using states  = types::join_t<O<S, D>, typename parser<O, M, U...>::states>;
        using inputs  = types::join_t<O<I...>, typename parser<O, M, U...>::inputs>;
        using matches = types::join_t<O<O<S, I>...>, typename parser<O, M, U...>::matches>;
    };

public:
    using action  = typename collect<T...>::action;
    using states  = types::distinct_stable_t<typename collect<T...>::states>;
    using inputs  = types::distinct_stable_t<typename collect<T...>::inputs>;
    using matches = typename collect<T...>::matches;
};

}}} // namespace uncat::fsm::aux

namespace uncat { namespace fsm {

template <typename S, typename D, typename... I> struct transition
{};

template <typename F, typename... T> struct state_machine
{
public:
    static_assert((aux::validator<F, T>::value && ...), "F may not support some state-input pairs");

private:
    using parser = aux::parser<types::pack, F, T...>;
    using states = typename parser::states;
    using inputs = typename parser::inputs;

public:
    template <typename S, typename X>
    state_machine(S && state, X && action)
        requires types::in<S, states>;
    state_machine();

    state_machine(state_machine const &)             = default;
    state_machine(state_machine &&)                  = default;
    state_machine & operator=(state_machine const &) = default;
    state_machine & operator=(state_machine &&)      = default;

public:
    template <typename I>
    auto accept(I && input) -> bool
        requires types::in<I, inputs>;

private:
    using holder = types::map_t<std::variant, states>;
    holder state;
    F      shift;
};

template <typename F, typename... T>
template <typename S, typename X>
inline state_machine<F, T...>::state_machine(S && init, X && next)
    requires types::in<S, states>
    : state(std::forward<S>(init))
    , shift(std::forward<X>(next))
{}

template <typename F, typename... T>
inline state_machine<F, T...>::state_machine()
    : state(types::map_t<types::first_t, states>())
    , shift(F())
{}

template <typename F, typename... T>
template <typename I>
inline auto state_machine<F, T...>::accept(I && input) -> bool
    requires types::in<I, inputs>
{
    return std::visit(
        [this, &input](auto & current) -> bool
        {
            using state_t = std::remove_cvref_t<decltype(current)>;
            using input_t = std::remove_cvref_t<I>;
            using match_t = typename parser::matches;

            using types::pack;
            auto constexpr acceptable = types::in<pack<state_t, input_t>, match_t>;
            if constexpr (acceptable)
                state = shift(current, std::forward<I>(input));
            return acceptable;
        },
        state
    );
}
}} // namespace uncat::fsm
