#pragma once
#include <deque>
#include <vector>
#include <type_traits>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace uncat
{
    struct world_line
    {
    private:
        using task_type = std::function<void()>;

    public:
        template<typename T>
        using bool_t = std::enable_if_t
            < std::is_assignable_v<task_type, T>
            , bool
            >;

    public:
        template<typename T>
        bool_t<T> cross(T && task);

    public:
         world_line(std::size_t number_of_lines = 1);
        ~world_line();

    private:
        void one_for_all();
        void one_for_one();

    private:
        bool                     running;
        std::deque<task_type>    tasks;
        std::vector<std::thread> workers;
        std::condition_variable  condition;
        std::mutex               mutex;
    };

    template<typename T>
    inline world_line::bool_t<T> world_line::cross(T && task)
    {
        auto o = true;
        {
            std::scoped_lock _(mutex);
            if ((o = running, o))
                tasks.push_back(std::forward<T>(task));
        }

        if (o)
            condition.notify_one();
        return o;
    }

    inline world_line::world_line(std::size_t number_of_lines)
        : running(true)
        , tasks()
        , workers()
        , condition()
        , mutex()
    {
        if         (number_of_lines ==  0)
            running = false;
        else if    (number_of_lines ==  1)
            workers.emplace_back(&world_line::one_for_all, this);
        else while (number_of_lines --> 0)
            workers.emplace_back(&world_line::one_for_one, this);
    }

    inline world_line::~world_line()
    {
        {
            std::scoped_lock _(mutex);
            running = false;
        }
        condition.notify_all();
        for (auto & worker : workers)
            worker.join();
    }

    inline void world_line::one_for_all()
    {
        auto todo = decltype(tasks)();
        do {
            todo.clear();
            {
                auto lock = std::unique_lock(mutex);
                condition.wait(lock, [&] { return !tasks.empty() || !running; });
                std::swap(todo, tasks);
            }
            for (auto & func : todo)
                func();
        }
        while (!todo.empty());
    }

    inline void world_line::one_for_one()
    {
        auto runn = true;
        auto work = false;
        while (runn || work)
        {
            auto todo = task_type();
            {
                auto lock = std::unique_lock(mutex);
                condition.wait(lock, [&] { return !tasks.empty() || !running; });

                runn = running;
                work = !tasks.empty();

                if (work)
                {
                    todo = std::move(tasks.front());
                    tasks.pop_front();
                }
            }

            if (work)
                todo();
        }
    }
}
