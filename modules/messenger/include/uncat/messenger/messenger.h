#pragma once
#include <condition_variable>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>

#include <uncat/messenger/executor.h>
#include <uncat/types/concepts.h>

namespace uncat
{

template <
    typename K,    // key    type, used as key in hash table
    typename... E> // events type, must be copyable and movable
class messenger
{
public:
    using name_t                          = K;
    template <typename T> using handler_t = std::function<void(T const &)>;
    template <typename T> using group_t   = std::unordered_map<name_t, handler_t<T>>;

public:
    template <types::oneof<E...> T> auto add_handler(name_t name, handler_t<T> handler) -> messenger &;

    template <types::oneof<E...> T> auto remove_handler(name_t name) -> messenger &;

    template <typename T>
    requires types::oneof<std::remove_cvref_t<T>, E...>
    auto send(T && event) -> messenger &;

public:
    auto wait() -> void;

private:
    template <typename T> auto add_handler_unsafe(name_t const & name, handler_t<T> const & func) -> void;
    template <typename T> auto remove_handler_unsafe(name_t const & name) -> void;
    template <typename T> auto send_unsafe(T const & event) -> void;

private:
    std::tuple<group_t<E>...> gorups; /// handlers is only used in one thread.
    exec::executor            todo{1};
};

template <typename K, typename... E>
template <types::oneof<E...> T>
inline auto messenger<K, E...>::add_handler(name_t name, handler_t<T> func) -> messenger &
{
    todo([this, name = std::move(name), func = std::move(func)]
         { add_handler_unsafe<T>(std::move(name), std::move(func)); });
    return *this;
}

template <typename K, typename... E>
template <types::oneof<E...> T>
inline auto messenger<K, E...>::remove_handler(name_t name) -> messenger &
{
    todo([this, name = std::move(name)] { remove_handler_unsafe<T>(std::move(name)); });
    return *this;
}

template <typename K, typename... E>
template <typename T>
requires types::oneof<std::remove_cvref_t<T>, E...>
inline auto messenger<K, E...>::send(T && event) -> messenger &
{
    todo([this, event = std::move(event)] { send_unsafe<std::remove_cvref_t<T>>(event); });
    return *this;
}

template <typename K, typename... E> inline auto messenger<K, E...>::wait() -> void
{
    auto sign = std::make_shared<std::promise<void>>();
    auto wait = sign->get_future();
    auto done = todo([sign = std::move(sign)] { sign->set_value(); });

    if (done)
        wait.get();
}

template <typename K, typename... E>
template <typename T>
inline auto messenger<K, E...>::add_handler_unsafe(name_t const & name, handler_t<T> const & func) -> void
{
    auto & group = std::get<group_t<T>>(gorups);
    group[name]  = func;
}

template <typename K, typename... E>
template <typename T>
inline auto messenger<K, E...>::remove_handler_unsafe(name_t const & name) -> void
{
    auto & group = std::get<group_t<T>>(gorups);
    auto   iter  = group.find(name);
    auto   find  = iter != group.end();

    if (find)
        group.erase(iter);
}

template <typename K, typename... E>
template <typename T>
inline auto messenger<K, E...>::send_unsafe(T const & event) -> void
{
    for (auto & pair : std::get<group_t<T>>(gorups))
        pair.second(event);
}

} // namespace uncat
