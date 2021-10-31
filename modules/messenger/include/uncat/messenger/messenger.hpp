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
#include <uncat/exec/executor.hpp>
#include <uncat/types/types.hpp>

namespace uncat
{
    template
        < typename    K  // key    type, used as key in hash table
        , typename ...T  // events type, must be copyable and movable
        > class messenger
    {
    public:
        using name_t = K;
        template<typename E> using handler_t = std::function<void(E const&)>;
        template<typename E> using   group_t = std::unordered_map<name_t, handler_t<E>>;
        template<typename E> using   event_t =  find_t<remove_cvr_t<E>, T...>;
        template<typename E> using    this_t = first_t<messenger, event_t<E>>;

    public:
        template<typename E> this_t<E> &    add_handler(name_t name, handler_t<E> handler);
        template<typename E> this_t<E> & remove_handler(name_t name);
        template<typename E> this_t<E> &           send(E && event);

    public:
        void wait();

    private:
        template<typename E> void    add_handler_unsafe(name_t const & name, handler_t<E> const & handler);
        template<typename E> void remove_handler_unsafe(name_t const & name);
        template<typename E> void           send_unsafe(E const & event);

    private:
        std::tuple<group_t<T>...> gorups; /// handlers is only used in one thread.
        exec::executor            todo{1};
    };

    template<typename K, typename ...T>
    template<typename U> inline
    typename messenger<K, T...>::template this_t<U> & messenger<K, T...>::
    add_handler(name_t name, handler_t<U> func)
    {
        todo([this, name = std::move(name), func = std::move(func)]
        {
            add_handler_unsafe<U>(std::move(name), std::move(func));
        });
        return *this;
    }

    template<typename K, typename ...T>
    template<typename U> inline
    typename messenger<K, T...>::template this_t<U> & messenger<K, T...>::
    remove_handler(name_t name)
    {
        todo([this, name = std::move(name)]
        {
            remove_handler_unsafe<U>(std::move(name));
        });
        return *this;
    }

    template<typename K, typename ...T>
    template<typename U> inline
    typename messenger<K, T...>::template this_t<U> & messenger<K, T...>::
    send(U && event)
    {
        todo([this, event = std::move(event)]
        {
            send_unsafe<remove_cvr_t<U>>(event);
        });
        return *this;
    }

    template<typename K, typename ...T> inline
    void messenger<K, T...>::
    wait()
    {
        auto sign = std::make_shared<std::promise<void>>();
        auto wait = sign->get_future();
        auto done = todo([sign = std::move(sign)]
        {
            sign->set_value();
        });

        if (done)
            wait.get();
    }

    template<typename K, typename ...T>
    template<typename U> inline void messenger<K, T...>::
    add_handler_unsafe(name_t const & name, handler_t<U> const & func)
    {
        auto& group = std::get<group_t<U>>(gorups);
        group[name] = func;
    }

    template<typename K, typename ...T>
    template<typename U> inline void messenger<K, T...>::
    remove_handler_unsafe(name_t const & name)
    {
        auto & group = std::get<group_t<U>>(gorups);
        auto    iter = group.find(name);
        auto    find = iter != group.end();

        if (find)
            group.erase(iter);
    }

    template<typename K, typename ...T>
    template<typename U> inline void messenger<K, T...>::
    send_unsafe(U const & event)
    {
        for (auto & pair : std::get<group_t<U>>(gorups))
            pair.second(event);
    }
}
