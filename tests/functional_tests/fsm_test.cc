#include <catch2/catch_test_macros.hpp>

#include <uncat/fsm/state_machine.h>

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
    inline unlocked_state operator()(locked_state & /*unused*/, coin_input const & /*unused*/)
    {
        return {};
    }

    /// unlocked_state --push_input--> locked_state
    inline locked_state operator()(unlocked_state & /*unused*/, push_input /*unused*/)
    {
        return {};
    }

    /// unexpected transition, should be ignored
    inline unlocked_state operator()(locked_state & /*unused*/, push_input const & /*unused*/)
    {
        return {};
    }

    /// unexpected transition, should be ignored
    inline locked_state operator()(unlocked_state & /*unused*/, coin_input /*unused*/)
    {
        return {};
    }
};

TEST_CASE("coin operated turnstile", "[fsm]")
{
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
            REQUIRE(m.accept(push_input()) == false);
            REQUIRE(m.accept(coin_input()) == true);
            REQUIRE(m.accept(coin_input()) == false);
            REQUIRE(m.accept(push_input()) == true);
        }
    }
    SECTION("constructor with initial values")
    {
        auto m = turnstile_state_machine(unlocked_state(), coin_operated_turnstile());
        for (auto i = 0; i < 3; ++i)
        {
            REQUIRE(m.accept(push_input()) == true);
            REQUIRE(m.accept(push_input()) == false);
            REQUIRE(m.accept(coin_input()) == true);
            REQUIRE(m.accept(coin_input()) == false);
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
        REQUIRE(m.accept(coin_input()) == true);
    }
}
