#include <catch2/catch_test_macros.hpp>

#include <uncat/fsm/state_machine.h>

TEST_CASE("coin operated turnstile", "[fsm]")
{
    struct locked_state
    {};

    struct unlocked_state
    {};

    struct push_input
    {};

    struct coin_input
    {};

    struct coin_operated_turnstile
    {
        /// locked_state --coin_input--> unlocked_state
        inline auto operator()(locked_state & /*unused*/, coin_input const & /*unused*/) -> unlocked_state
        {
            return {};
        }

        /// unlocked_state --push_input--> locked_state
        inline auto operator()(unlocked_state & /*unused*/, push_input /*unused*/) -> locked_state
        {
            return {};
        }
    };

    using uncat::fsm::state_machine, uncat::fsm::transition;
    using turnstile_state_machine = state_machine<
        coin_operated_turnstile,                              // states
        transition<locked_state, unlocked_state, coin_input>, // transitions
        transition<unlocked_state, locked_state, push_input>>;

    SECTION("default constructor")
    {
        auto m = turnstile_state_machine();
        for (auto i = 0; i < 3; ++i)
        {
            REQUIRE(m.input(push_input()) == false);
            REQUIRE(m.input(coin_input()) == true);
            REQUIRE(m.input(coin_input()) == false);
            REQUIRE(m.input(push_input()) == true);
        }
    }
    SECTION("constructor with initial values")
    {
        auto m = turnstile_state_machine(unlocked_state(), coin_operated_turnstile());
        for (auto i = 0; i < 3; ++i)
        {
            REQUIRE(m.input(push_input()) == true);
            REQUIRE(m.input(push_input()) == false);
            REQUIRE(m.input(coin_input()) == true);
            REQUIRE(m.input(coin_input()) == false);
        }
    }
    SECTION("use as a member")
    {
        struct wrapper
        {
            turnstile_state_machine machine;
        };

        auto   w = wrapper();
        auto & m = w.machine;
        REQUIRE(m.input(coin_input()) == true);
    }
}
