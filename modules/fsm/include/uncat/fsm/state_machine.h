#pragma once
#include <type_traits>
#include <variant>

#include <uncat/types/concepts.h>
#include <uncat/types/types.h>

namespace uncat::fsm
{

template <typename F, typename T> auto constexpr is_transition = false;
template <typename F, template <typename...> class T, typename S, typename D, typename... I>
auto constexpr is_transition<F, T<S, D, I...>> =
    (std::is_invocable_r_v<D, F, std::add_lvalue_reference_t<S>, I> && ...);

template <typename F, typename T>
concept transitional = is_transition<F, T>;

} // namespace uncat::fsm

namespace uncat::fsm::aux
{

template <template <typename...> class O, typename F, typename... T>
requires(transitional<F, T> && ...)
struct parser
{
private:
    template <typename... U> struct collect
    {
        using states  = O<>;
        using inputs  = O<>;
        using matches = O<>;
    };

    template <template <typename...> class C, typename S, typename D, typename... I, typename... U>
    struct collect<C<S, D, I...>, U...>
    {
        using states  = types::join_t<O<S, D>, typename collect<U...>::states>;
        using inputs  = types::join_t<O<I...>, typename collect<U...>::inputs>;
        using matches = types::join_t<O<O<S, I>...>, typename collect<U...>::matches>;
    };

public:
    using action  = F;
    using states  = types::distinct_stable_t<typename collect<T...>::states>;
    using inputs  = types::distinct_stable_t<typename collect<T...>::inputs>;
    using matches = typename collect<T...>::matches;
};

} // namespace uncat::fsm::aux

namespace uncat::fsm
{

template <typename S, typename D, typename... I> struct transition
{};

template <typename F, typename... T>
requires(transitional<F, T> && ...)
struct state_machine
{
private:
    using parser = aux::parser<types::pack, F, T...>;
    using states = typename parser::states;
    using inputs = typename parser::inputs;

public:
    template <typename S, typename X>
    state_machine(S && initial_state, X && state_transition)
    requires types::in<S, states>
        : state(std::forward<S>(initial_state))
        , shift(std::forward<X>(state_transition))
    {}

    state_machine()
        : state(types::map_t<types::first_t, states>())
        , shift(F())
    {}

    template <typename I>
    auto accept(I && input) -> bool
    requires types::in<I, inputs>
    {
        return std::visit(
            [this, input = std::forward<I>(input)](auto & current) mutable -> bool
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

private:
    using holder = types::map_t<std::variant, states>;
    holder state;
    F      shift;
};

} // namespace uncat::fsm
