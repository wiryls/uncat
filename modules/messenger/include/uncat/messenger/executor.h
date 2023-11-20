#pragma once
#include <concepts>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <type_traits>
#include <vector>

namespace uncat::exec
{

class executor
{
public:
    explicit executor(std::size_t size = std::thread::hardware_concurrency());
    ~executor();
    executor(executor && rhs)                  = delete;
    executor(executor const & rhs)             = delete;
    executor & operator=(executor && rhs)      = delete;
    executor & operator=(executor const & rhs) = delete;

public:
    using task = std::function<void()>;
    auto operator()(std::convertible_to<executor::task> auto && todo) -> bool;

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

auto executor::operator()(std::convertible_to<executor::task> auto && todo) -> bool
{
    return push(task(std::forward<decltype(todo)>(todo)));
}

} // namespace uncat::exec
