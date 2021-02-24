#pragma once
#include <list>
#include <type_traits>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace pw
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
        bool_t<T> enqueue(T && task);

    public:
         world_line(std::size_t number_of_lines = 1);
        ~world_line();

    private:
        void one_for_all();
        void all_for_all();

    private:
        bool                    running;
        std::list<task_type>    tasks;
        std::list<std::thread>  workers;
        std::condition_variable condition;
        std::mutex              lock;
    };

    template<typename T>
    inline world_line::bool_t<T> world_line::enqueue(T && task)
    {
        auto o = true;
        {
            std::lock_guard<std::mutex> _(lock);
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
        , lock()
    {
        if /*___*/ (number_of_lines == 1)
            workers.emplace_back(&world_line::one_for_all, this);
        else while (number_of_lines != 0 && number_of_lines --> 0)
            workers.emplace_back(&world_line::all_for_all, this);
    }

    inline world_line::~world_line()
    {
        {
            std::lock_guard<std::mutex> _(lock);
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
                auto ul = std::unique_lock<std::mutex>(lock);
                condition.wait(ul, [&] { return !tasks.empty() || !running; });
                std::swap(todo, tasks);
            }
            for (auto & f : todo)
                f();
        }
        while (!todo.empty());
    }

    inline void world_line::all_for_all()
    {
        auto runn = true;
        auto work = false;
        while (runn || work)
        {
            auto todo = task_type();
            {
                auto ul = std::unique_lock<std::mutex>(lock);
                condition.wait(ul, [&] { return !tasks.empty() || !running; });

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
