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
        using task_t = std::function<void()>;

    public:
        template<typename T>
        using bool_t = std::enable_if_t<std::is_assignable_v<task_t, T>, bool>;

    public:
        template<typename T> bool_t<T> cross(T && task);

    public:
        explicit world_line(std::size_t size = 1);
                ~world_line();

    private:
        void one_for_all();
        void one_for_one();

    private:
        bool                     running;
        std::deque<task_t>       points;
        std::vector<std::thread> runners;
        std::condition_variable  condition;
        std::mutex               mutex;
    };

    template<typename T> inline
    world_line::bool_t<T> world_line::cross(T && task)
    {
        auto o = true;
        {
            std::scoped_lock _(mutex);
            if ((o = running, o))
                points.push_back(std::forward<T>(task));
        }

        if (o)
            condition.notify_one();
        return o;
    }

    inline world_line::world_line(std::size_t size)
        : running(true)
        , points()
        , runners()
        , condition()
        , mutex()
    {
        runners.reserve(size);
        if         (size ==  0)
            running = false;
        else if    (size ==  1)
            runners.emplace_back(&world_line::one_for_all, this);
        else while (size --> 0)
            runners.emplace_back(&world_line::one_for_one, this);
    }

    inline world_line::~world_line()
    {
        {
            std::scoped_lock _(mutex);
            running = false;
        }
        condition.notify_all();
        for (auto & runner : runners)
            runner.join();
    }

    inline void world_line::one_for_all()
    {
        auto todo = decltype(points)();
        do {
            todo.clear();
            {
                auto lock = std::unique_lock(mutex);
                condition.wait(lock, [&] { return !points.empty() || !running; });
                std::swap(todo, points);
            }
            for (auto & func : todo)
                func();
        }
        while (!todo.empty());
    }

    inline void world_line::one_for_one()
    {
        auto loop = true;
        auto grab = false;
        while (loop || grab)
        {
            auto todo = task_t();
            {
                auto lock = std::unique_lock(mutex);
                condition.wait(lock, [&] { return !points.empty() || !running; });

                loop = running;
                grab = !points.empty();

                if (grab)
                {
                    todo = std::move(points.front());
                    points.pop_front();
                }
            }

            if (grab)
                todo();
        }
    }
}
