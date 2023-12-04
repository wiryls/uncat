#pragma once
#include <optional>
#include <type_traits>
#include <variant>

#include <uncat/fsm/common.h>

namespace uncat::fsm::aux
{

// An unary predicate builder to match tails.
template <typename... T> struct with_tail
{
    template <typename V> struct type
    {
        auto static constexpr value = false;
    };

    template <template <typename...> class L, typename V> struct type<L<V /* make clang happy */, T...>>
    {
        auto static constexpr value = true;
    };
};

// Used to support for custom types.
template <typename T> struct state_like
{};

// An example to support std::optional
template <typename T> struct state_like<std::optional<T>>
{
    using type = T;

    auto inline static constexpr ready(std::optional<T> const & o) -> bool
    {
        return o.has_value();
    }

    auto inline static constexpr value(std::optional<T> && o) -> T
    {
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        return o.value();
    }
};

// Check wether T::type is R or T::type can be converted to R.
template <typename T, typename R> auto constexpr match_state = false;
template <type_member T /* make msvc happy */, typename R>
auto constexpr match_state<T, R> = std::is_same_v<R, typename T::type>;
template <typename T, typename R>
requires type_member<T> && type_member<state_like<typename T::type>>
auto constexpr match_state<T, R> = std::is_same_v<R, typename state_like<typename T::type>::type>;

// Define how to transition. Try to invoke F with S& and I.
template <typename F, typename S, typename I>
using transition_result = std::invoke_result<F, std::add_lvalue_reference_t<S>, I>;

// Check wether T can be transitioned by F.
template <typename F, typename T> auto constexpr is_transition = false;
template <typename F, template <typename...> class T, typename S, typename D, typename... I>
auto constexpr is_transition<F, T<S, D, I...>> =
    (sizeof...(I) > 0) && (match_state<transition_result<F, S, I>, D> && ...);

} // namespace uncat::fsm::aux

namespace uncat::fsm
{

template <typename Function, typename Transition>
concept transitional = aux::is_transition<Function, Transition>;

template <typename From, typename To, typename... Inputs> struct transition
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
            using matches = typename collect<U...>::matches::template push_front<aux::list<D, S, I>...>;
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
    requires std::default_initializable<F> && std::default_initializable<typename states::first>
        : state(typename states::first())
        , shift(F())
    {}

    template <typename I>
    requires(inputs::template contains<I>)
    auto input(I && event) -> bool // NOLINT(cppcoreguidelines-missing-std-forward)
    {
        return std::visit(
            [this, &event](auto & current) mutable -> bool
            {
                using state_t = std::remove_cvref_t<decltype(current)>;
                using input_t = std::remove_cvref_t<I>;
                using query_t = aux::with_tail<state_t, input_t>; /* make msvc happy */
                using match_t = parser::matches::template find<query_t::template type>;
                if constexpr (aux::type_member<match_t> && requires { typename match_t::type::first; })
                {
                    using result_t = aux::transition_result<F, state_t, input_t>;
                    if constexpr (aux::type_member<result_t>)
                    {
                        using value_type = typename result_t::type;
                        using operation  = aux::state_like<value_type>;
                        if constexpr (aux::same<value_type, typename match_t::type::first>)
                        {
                            state = shift(current, std::forward<I>(event));
                            return true;
                        }
                        else if constexpr (aux::type_member<operation>)
                        {
                            auto value = shift(current, std::forward<I>(event));
                            auto ready = operation::ready(value);
                            if (ready)
                                state = operation::value(std::move(value));
                            return ready;
                        }
                    }
                }
                else /* make msvc happy */
                {
                    return false;
                }
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
