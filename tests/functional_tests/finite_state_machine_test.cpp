#include <catch2/catch_test_macros.hpp>
#include <uncat/finite_state_machine.hpp>

struct   locked_state {};
struct unlocked_state {};
struct    push_action {};
struct    coin_action {};

template<typename S, typename I> struct coin_operated_turnstile {};
template<> struct coin_operated_turnstile<locked_state, coin_action>
{
    unlocked_state operator()(locked_state &, coin_action const &)
    {
        return unlocked_state();
    }
};
template<> struct coin_operated_turnstile<unlocked_state, push_action>
{
    locked_state operator()(unlocked_state&, push_action)
    {
        return locked_state();
    }
};

TEST_CASE("todo", "[finite_state_machine_test]")
{
    using uncat::state_machine;
    using uncat::transition;

    auto m = state_machine
        < coin_operated_turnstile
        , transition<locked_state, unlocked_state, coin_action>
        , transition<unlocked_state, locked_state, push_action>
    >();

    REQUIRE(m.accept(push_action()) == false);
    REQUIRE(m.accept(coin_action()) == true);
    REQUIRE(m.accept(coin_action()) == false);
    REQUIRE(m.accept(push_action()) == true);
    REQUIRE(m.accept(push_action()) == false);
}
