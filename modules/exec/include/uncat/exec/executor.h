#pragma once
#include <concepts>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <type_traits>
#include <vector>

namespace uncat
{
namespace exec
{

class executor
{
public:
    using task = std::function<void()>;

    template <std::convertible_to<executor::task> T>
    auto operator()(T && todo) -> bool;

public:
    explicit executor(std::size_t size = std::thread::hardware_concurrency());
    ~executor();

private:
    auto push(task && todo) -> bool;
    auto one_for_all() -> void;
    auto one_for_one() -> void;

private:
    bool                     running;
    std::deque<task>         tasks;
    std::mutex               mutex;
    std::condition_variable  condition;
    std::vector<std::thread> consumers;
};

template <std::convertible_to<executor::task> T>
auto executor::operator()(T && todo) -> bool
{
    return push(std::move(static_cast<task>(std::forward<T>(todo))));
}

} // namespace exec
} // namespace uncat
