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
        < typename ...T
        > class messenger : private types
    {
    public:
        using name_t = void const *;
        template<typename M> using contact_t = std::function<void(M const&)>;
        template<typename M> using   group_t = std::unordered_map<name_t, contact_t<M>>;
        template<typename M> using message_t = find_t<std::remove_cv_t<std::remove_reference_t<M>>, T...>;
        template<typename M> using    void_t = last_t<message_t<M>, void>;
        template<typename M, bool C> using maybe_bool_t = last_t<message_t<M>, select_t<C, bool, void>>;

    public:
        template<typename M, bool S = false> maybe_bool_t<M, S>    add_contact(name_t name, contact_t<M> f);
        template<typename M, bool S = false> maybe_bool_t<M, S> remove_contact(name_t name);
        template<typename M>                       void_t<M>              send(M && message);

    public:
         messenger();
        ~messenger();

    private:
        template<typename M> bool add_contact_sync  (name_t name, contact_t<M> && contact);
        template<typename M> void add_contact_async (name_t name, contact_t<M> && contact);
        template<typename M> bool add_contact_unsafe(name_t name, contact_t<M>    contact);

        template<typename M> bool remove_contact_sync  (name_t name);
        template<typename M> void remove_contact_async (name_t name);
        template<typename M> bool remove_contact_unsafe(name_t name);

        template<typename M> void send_async (M      && message);
        template<typename M> void send_unsafe(M const & message);

    private:
        std::tuple<group_t<T>...> gorups; /// handlers is only used in one thread.
        world_line                todo;
    };

    template<typename ...T>
    template<typename U, bool S> inline
    typename messenger<T...>::template maybe_bool_t<U, S> messenger<T...>::
    add_contact(name_t name, contact_t<U> callback)
    {
        if constexpr (S)
            return add_contact_sync <U>(name, std::move(callback));
        else
            /*  */ add_contact_async<U>(name, std::move(callback));
    }

    template<typename ...T>
    template<typename U, bool S> inline
    typename messenger<T...>::template maybe_bool_t<U, S> messenger<T...>::
    remove_contact(name_t name)
    {
        if constexpr (S)
            return remove_contact_sync <U>(name);
        else
            /*  */ remove_contact_async<U>(name);
    }

    template<typename ...T>
    template<typename U> inline
    typename messenger<T...>::template void_t<U> messenger<T...>::
    send(U && message)
    {
        send_async(std::forward<U>(message));
    }

    template<typename ...T> inline messenger<T...>::
    messenger()
        : gorups()
        , todo()
    {}

    template<typename ...T> inline messenger<T...>::
    ~messenger()
    {}

    template<typename ...T>
    template<typename U> inline bool messenger<T...>::
    add_contact_sync(name_t name, contact_t<U> && func)
    {
        auto done = static_cast<bool>(func);
        if (done)
        {
            auto sign = std::make_shared<std::promise<void>>();
            auto wait = sign->get_future();

            done = todo.push([this, name, sign = std::move(sign), func = std::move(func)]
            {
                add_contact_unsafe<U>(name, func);
                sign->set_value();
            });

            if (done)
                wait.get();
        }
        return done;
    }

    template<typename ...T>
    template<typename U> inline void messenger<T...>::
    add_contact_async(name_t name, contact_t<U> && func)
    {
        todo.cross([this, name, func = std::move(func)]
        {
            add_contact_unsafe<U>(name, func);
        });
    }

    template<typename ...T>
    template<typename U> inline bool messenger<T...>::
    add_contact_unsafe(name_t name, contact_t<U> func)
    {
        auto& group = std::get<group_t<U>>(gorups);
        group[name] = std::move(func);
        return true;
    }

    template<typename ...T>
    template<typename U> inline bool messenger<T...>::
    remove_contact_sync(name_t name)
    {
        auto sign = std::make_shared<std::promise<void>>();
        auto wait = sign->get_future();
        auto done = todo.cross([this, name, sign = std::move(sign)]
        {
            remove_contact_unsafe<U>(name);
            sign->set_value();
        });

        if (done)
            wait.get();

        return done;
    }

    template<typename ...T>
    template<typename U> inline void messenger<T...>::
    remove_contact_async(name_t name)
    {
        todo.cross([this, name]
        {
            remove_contact_unsafe<U>(name);
        });
    }

    template<typename ...T>
    template<typename U> inline bool messenger<T...>::
    remove_contact_unsafe(name_t name)
    {
        auto & group = std::get<group_t<U>>(gorups);
        auto    iter = group.find(name);
        auto    find = iter != group.end();
        if (find)
            group.erase(iter);

        return find;
    }

    template<typename ...T>
    template<typename U> inline void messenger<T...>::
    send_async(U && message)
    {
        todo.cross([this, message = std::move(message)]
        {
            send_unsafe<std::remove_cv_t<std::remove_reference_t<U>>>(message);
        });
    }

    template<typename ...T>
    template<typename U> inline void messenger<T...>::
    send_unsafe(U const & message)
    {
        for (auto & pair : std::get<group_t<U>>(gorups))
            pair.second(message);
    }
}
