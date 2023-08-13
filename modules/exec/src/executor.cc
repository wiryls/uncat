#include <uncat/exec/executor.h>

uncat::exec::executor::executor(std::size_t size)
    : running(size > 0)
    , tasks()
    , mutex()
    , condition()
    , consumers()
{
    consumers.reserve(size);
    if (size == 0)
        running = false;
    else if (size == 1)
        consumers.emplace_back(&executor::one_for_all, this);
    else
        while (size-- > 0)
            consumers.emplace_back(&executor::one_for_one, this);
}

uncat::exec::executor::~executor()
{
    {
        std::scoped_lock _(mutex);
        running = false;
    }

    condition.notify_all();
    for (auto & executor : consumers)
        executor.join();
}

auto uncat::exec::executor::push(task && todo) -> bool
{
    auto o = true;
    {
        std::scoped_lock _(mutex);
        if (o = running; o)
            tasks.push_back(std::move(todo));
    }

    if (o)
        condition.notify_one();

    return o;
}

auto uncat::exec::executor::one_for_one() -> void
{
    auto keep = true;
    while (keep)
    {
        auto todo = task();
        {
            auto lock = std::unique_lock(mutex);
            condition.wait(lock, [&] { return !tasks.empty() || !running; });

            if (tasks.empty())
            {
                keep = running;
            }
            else
            {
                // maybe replace it with a lock-free queue some day.
                todo = std::move(tasks.front());
                tasks.pop_front();
            }
        }

        if (todo)
            todo();
    }
}

auto uncat::exec::executor::one_for_all() -> void
{
    auto todo = decltype(tasks)();
    do
    {
        todo.clear();
        {
            auto lock = std::unique_lock(mutex);
            condition.wait(lock, [&] { return !tasks.empty() || !running; });
            std::swap(todo, tasks);
        }
        for (auto & func : todo)
            func();
    } while (!todo.empty());
}
