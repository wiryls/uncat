#pragma once
#include <type_traits>
#include <variant>

#include <uncat/fsm/list.h>

namespace uncat::fsm
{

template <typename F, typename T> auto constexpr is_transition = false;
template <typename F, template <typename...> class T, typename S, typename D, typename... I>
auto constexpr is_transition<F, T<S, D, I...>> =
    (std::is_invocable_r_v<D, F, std::add_lvalue_reference_t<S>, I> && ...);

template <typename F, typename T>
concept transitional = is_transition<F, T>;

template <typename S, typename D, typename... I> struct transition
{};

template <typename F, typename... T>
requires(transitional<F, T> && ...)
struct state_machine
{
private:
    struct parser
    {
        template <typename... U> struct collect
        {
            using states  = aux::list<>;
            using inputs  = aux::list<>;
            using matches = aux::list<>;
        };

        template <template <typename...> class C, typename S, typename D, typename... I, typename... U>
        struct collect<C<S, D, I...>, U...>
        {
            using states  = typename collect<U...>::states::template push_front<S, D>;
            using inputs  = typename collect<U...>::inputs::template push_front<I...>;
            using matches = typename collect<U...>::matches::template push_front<aux::list<S, I>...>;
        };

        using states  = typename aux::distinct_stable<typename collect<T...>::states>::type;
        using inputs  = typename aux::distinct_stable<typename collect<T...>::inputs>::type;
        using matches = typename collect<T...>::matches;
    };

    using states = typename parser::states;
    using inputs = typename parser::inputs;

public:
    template <typename S, typename X>
    requires(states::template contains<S>)
    explicit state_machine(S && initial_state, X && state_transition)
        : state(std::forward<S>(initial_state))
        , shift(std::forward<X>(state_transition))
    {}

    state_machine()
    requires requires { typename states::front; }
        : state(typename states::front())
        , shift(F())
    {}

    template <typename I>
    requires(inputs::template contains<I>)
    auto accept(I && input) -> bool // NOLINT(cppcoreguidelines-missing-std-forward)
    {
        return std::visit(
            [this, &input](auto & current) mutable -> bool
            {
                using state_t = std::remove_cvref_t<decltype(current)>;
                using input_t = std::remove_cvref_t<I>;
                using match_t = aux::list<state_t, input_t>;

                auto constexpr acceptable = parser::matches::template contains<match_t>;
                if constexpr (acceptable)
                    state = shift(current, std::forward<I>(input));
                return acceptable;
            },
            state
        );
    }

private:
    using holder = typename states::template as<std::variant>;
    holder state;
    F      shift;
};

} // namespace uncat::fsm
