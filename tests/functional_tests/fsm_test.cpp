#include <catch2/catch_test_macros.hpp>
#include <uncat/fsm/state_machine.hpp>

struct   locked_state {};
struct unlocked_state {};
struct     push_input {};
struct     coin_input {};

struct coin_operated_turnstile
{
    /// locked_state --coin_input--> unlocked_state
    inline unlocked_state operator()(  locked_state&, coin_input const&)
    {
        return unlocked_state();
    }
    /// unlocked_state --push_input--> locked_state
    inline   locked_state operator()(unlocked_state&, push_input)
    {
        return locked_state();
    }

    /// unexpected transition, should be ignored
    inline unlocked_state operator()(  locked_state&, push_input const&)
    {
        return unlocked_state();
    }
    /// unexpected transition, should be ignored
    inline   locked_state operator()(unlocked_state&, coin_input)
    {
        return locked_state();
    }
};

TEST_CASE("coin operated turnstile", "[fsm]")
{
    using uncat::state_machine;
    using uncat::transition;

    SECTION("with default constructor")
    {
        auto m = state_machine
            < coin_operated_turnstile
            , transition<locked_state, unlocked_state, coin_input>
            , transition<unlocked_state, locked_state, push_input>
            >();

        for (auto i = 0; i < 3; ++i)
        {
            REQUIRE(m.accept(push_input()) == false);
            REQUIRE(m.accept(coin_input()) == true);
            REQUIRE(m.accept(coin_input()) == false);
            REQUIRE(m.accept(push_input()) == true);
        }
    }
    SECTION("with another constructor")
    {
        auto m = state_machine
            < coin_operated_turnstile
            , transition<locked_state, unlocked_state, coin_input>
            , transition<unlocked_state, locked_state, push_input>
            >(unlocked_state(), coin_operated_turnstile());

        for (auto i = 0; i < 3; ++i)
        {
            REQUIRE(m.accept(push_input()) == true);
            REQUIRE(m.accept(push_input()) == false);
            REQUIRE(m.accept(coin_input()) == true);
            REQUIRE(m.accept(coin_input()) == false);
        }
    }
    SECTION("as a member variable")
    {
        struct wrapper
        {
            state_machine
                < coin_operated_turnstile
                , transition<locked_state, unlocked_state, coin_input>
                , transition<unlocked_state, locked_state, push_input>
                > machine;
        };

        auto   w = wrapper();
        auto & m = w.machine;
        REQUIRE(m.accept(coin_input()) == true);
    }
}
