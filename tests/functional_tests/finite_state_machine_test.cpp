#include <catch2/catch_test_macros.hpp>
#include <uncat/finite_state_machine.hpp>

struct   locked_state {};
struct unlocked_state {};
struct    push_action {};
struct    coin_action {};

struct coin_operated_turnstile
{
    /// locked_state --coin_action--> unlocked_state
    inline unlocked_state operator()(  locked_state&, coin_action const&)
    {
        return unlocked_state();
    }
    /// unlocked_state --push_action--> locked_state
    inline   locked_state operator()(unlocked_state&, push_action)
    {
        return locked_state();
    }

    /// unexpected transitions, should be ignored
    inline unlocked_state operator()(  locked_state&, push_action const&)
    {
        return unlocked_state();
    }
    /// unexpected transitions, should be ignored
    inline   locked_state operator()(unlocked_state&, coin_action)
    {
        return locked_state();
    }
};

TEST_CASE("coin operated turnstile", "[finite_state_machine_test]")
{
    using uncat::state_machine;
    using uncat::transition;

    SECTION("with default constructor")
    {
        auto m = state_machine
            < coin_operated_turnstile
            , transition<locked_state, unlocked_state, coin_action>
            , transition<unlocked_state, locked_state, push_action>
            >();

        for (auto i = 0; i < 3; ++i)
        {
            REQUIRE(m.accept(push_action()) == false);
            REQUIRE(m.accept(coin_action()) == true);
            REQUIRE(m.accept(coin_action()) == false);
            REQUIRE(m.accept(push_action()) == true);
        }
    }

    SECTION("with another constructor")
    {
        auto m = state_machine
            < coin_operated_turnstile
            , transition<locked_state, unlocked_state, coin_action>
            , transition<unlocked_state, locked_state, push_action>
            >(unlocked_state(), coin_operated_turnstile());

        for (auto i = 0; i < 3; ++i)
        {
            REQUIRE(m.accept(push_action()) == true);
            REQUIRE(m.accept(push_action()) == false);
            REQUIRE(m.accept(coin_action()) == true);
            REQUIRE(m.accept(coin_action()) == false);
        }
    }

    SECTION("as a member variable")
    {
        struct wrapper
        {
            state_machine
                < coin_operated_turnstile
                , transition<locked_state, unlocked_state, coin_action>
                , transition<unlocked_state, locked_state, push_action>
                > machine;
        };

        auto   w = wrapper();
        auto & m = w.machine;
        REQUIRE(m.accept(coin_action()) == true);
    }
}
