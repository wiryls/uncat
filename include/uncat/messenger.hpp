#pragma once
#include <tuple>
#include <list>
#include <unordered_map>
#include <functional>
#include <memory>
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <uncat/parallel_world.hpp>
#include <uncat/detail/types.hpp>

namespace uncat
{
    template
        < typename    K  // key    type, used as key in hash table
        , typename ...T  // events type, must be copyable and movable
        > class messenger : private types
    {
    public:
        using name_t = K;
        template<typename E> using handler_t = std::function<void(E const&)>;
        template<typename E> using   group_t = std::unordered_map<name_t, handler_t<E>>;
        template<typename E> using   event_t = find_t<remove_cvr_t<E>, T...>;
        template<typename E> using    void_t = last_t<event_t<E>, void>;
        template<typename E, bool C> using maybe_bool_t = last_t<event_t<E>, select_t<C, bool, void>>;

    public:
        template<typename E, bool S = false> maybe_bool_t<E, S>    add_handler(name_t name, handler_t<E> f);
        template<typename E, bool S = false> maybe_bool_t<E, S> remove_handler(name_t name);
        template<typename E>                       void_t<E>              send(E && event);

    private:
        template<typename E> bool add_handler_sync  (name_t name, handler_t<E> && handler);
        template<typename E> void add_handler_async (name_t name, handler_t<E> && handler);
        template<typename E> bool add_handler_unsafe(name_t name, handler_t<E>    handler);

        template<typename E> bool remove_handler_sync  (name_t name);
        template<typename E> void remove_handler_async (name_t name);
        template<typename E> bool remove_handler_unsafe(name_t name);

        template<typename E> void send_async (E      && event);
        template<typename E> void send_unsafe(E const & event);

    private:
        std::tuple<group_t<T>...> gorups; /// handlers is only used in one thread.
        world_line                todo;
    };

    template<typename K, typename ...T>
    template<typename U, bool S> inline
    typename messenger<K, T...>::template maybe_bool_t<U, S> messenger<K, T...>::
    add_handler(name_t name, handler_t<U> handler)
    {
        if constexpr (S)
            return add_handler_sync <U>(name, std::move(handler));
        else
            /*  */ add_handler_async<U>(name, std::move(handler));
    }

    template<typename K, typename ...T>
    template<typename U, bool S> inline
    typename messenger<K, T...>::template maybe_bool_t<U, S> messenger<K, T...>::
    remove_handler(name_t name)
    {
        if constexpr (S)
            return remove_handler_sync <U>(name);
        else
            /*  */ remove_handler_async<U>(name);
    }

    template<typename K, typename ...T>
    template<typename U> inline
    typename messenger<K, T...>::template void_t<U> messenger<K, T...>::
    send(U && event)
    {
        send_async(std::forward<U>(event));
    }

    template<typename K, typename ...T>
    template<typename U> inline bool messenger<K, T...>::
    add_handler_sync(name_t name, handler_t<U> && func)
    {
        auto done = static_cast<bool>(func);
        if (done)
        {
            auto sign = std::make_shared<std::promise<bool>>();
            auto wait = sign->get_future();

            done = todo.cross([this, name, sign = std::move(sign), func = std::move(func)]
            {
                auto o = add_handler_unsafe<U>(name, func);
                sign->set_value(o);
            });

            if (done)
                done = wait.get();
        }
        return done;
    }

    template<typename K, typename ...T>
    template<typename U> inline void messenger<K, T...>::
    add_handler_async(name_t name, handler_t<U> && func)
    {
        todo.cross([this, name, func = std::move(func)]
        {
            add_handler_unsafe<U>(name, func);
        });
    }

    template<typename K, typename ...T>
    template<typename U> inline bool messenger<K, T...>::
    add_handler_unsafe(name_t name, handler_t<U> func)
    {
        auto& group = std::get<group_t<U>>(gorups);
        group[name] = std::move(func);
        return true;
    }

    template<typename K, typename ...T>
    template<typename U> inline bool messenger<K, T...>::
    remove_handler_sync(name_t name)
    {
        auto sign = std::make_shared<std::promise<bool>>();
        auto wait = sign->get_future();
        auto done = todo.cross([this, name, sign = std::move(sign)]
        {
            auto o = remove_handler_unsafe<U>(name);
            sign->set_value(o);
        });

        if (done)
            done = wait.get();

        return done;
    }

    template<typename K, typename ...T>
    template<typename U> inline void messenger<K, T...>::
    remove_handler_async(name_t name)
    {
        todo.cross([this, name]
        {
            remove_handler_unsafe<U>(name);
        });
    }

    template<typename K, typename ...T>
    template<typename U> inline bool messenger<K, T...>::
    remove_handler_unsafe(name_t name)
    {
        auto & group = std::get<group_t<U>>(gorups);
        auto    iter = group.find(name);
        auto    find = iter != group.end();
        if (find)
            group.erase(iter);

        return find;
    }

    template<typename K, typename ...T>
    template<typename U> inline void messenger<K, T...>::
    send_async(U && event)
    {
        todo.cross([this, event = std::move(event)]
        {
            send_unsafe<remove_cvr_t<U>>(event);
        });
    }

    template<typename K, typename ...T>
    template<typename U> inline void messenger<K, T...>::
    send_unsafe(U const & event)
    {
        for (auto & pair : std::get<group_t<U>>(gorups))
            pair.second(event);
    }
}
