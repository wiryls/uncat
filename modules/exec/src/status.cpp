#include <atomic>
#include <uncat/exec/status.hpp>

//// progress

struct uncat::exec::progress
{
private: // padding

    /* cache line size */
    auto inline constexpr static pad = std::size_t(64);

    /* helper to sum all */
    template<std::size_t ... N>
    auto inline constexpr static sum = (N + ...);

    /* padding type */
    template<std::size_t ... N>
    requires (pad > sum<N ...>)
    using padding = std::uint8_t[pad - sum<N ...>];

public: // members
    std::atomic<status>      state{ status::pending };
    std::atomic<std::size_t> refer{};
    padding<sizeof state, sizeof refer> __0{};

    std::atomic<std::size_t> count{};
    std::atomic<std::size_t> total{};
    padding<sizeof count, sizeof total> __1{};

public: // status
    auto inline status() const noexcept -> status;
    auto inline expire() const noexcept -> bool;
    auto inline holdon() const noexcept -> void;
    auto inline cancel()       noexcept -> void;

public: // refer && status
    auto inline increase() noexcept -> void;
    auto inline decrease() noexcept -> void;

public: // count && total
    auto inline now() const noexcept -> current;
    auto inline add(std::size_t delta) noexcept -> void;
    auto inline set(std::size_t delta) noexcept -> void;
    auto inline max(std::size_t delta) noexcept -> void;
};

auto uncat::exec::progress::status() const noexcept -> uncat::exec::status
{
    return state.load(std::memory_order_relaxed);
}

auto uncat::exec::progress::expire() const noexcept -> bool
{
    return state.load(std::memory_order_relaxed) != status::running;
}

auto uncat::exec::progress::holdon() const noexcept -> void
{
    state.wait(status::pending, std::memory_order_acquire);
    state.wait(status::running, std::memory_order_acquire);
    state.wait(status::halting, std::memory_order_acquire);
}

auto uncat::exec::progress::cancel() noexcept -> void
{
    auto current = status::running;
    state.compare_exchange_strong
        ( current
        , status::halting
        , std::memory_order_release
        , std::memory_order_relaxed );
}

auto uncat::exec::progress::increase() noexcept -> void
{
    // +: std::memory_order_relaxed
    // -: std::memory_order_acq_rel
    // https://stackoverflow.com/a/41425204
    if (refer.fetch_add(1, std::memory_order_relaxed) == 0)
    {
        auto current = status::pending;
        state.compare_exchange_strong
            ( current
            , status::running
            , std::memory_order_release
            , std::memory_order_relaxed );
    }
}

auto uncat::exec::progress::decrease() noexcept -> void
{
    // similar to sp: https://stackoverflow.com/a/891027
    if (refer.fetch_sub(1, std::memory_order_acq_rel) == 1)
    {
        state.store(status::stopped, std::memory_order_release);
        state.notify_all();
    }
}

auto uncat::exec::progress::now() const noexcept -> current
{
    return current
    {
        count.load(std::memory_order_relaxed),
        total.load(std::memory_order_relaxed),
    };
}

auto uncat::exec::progress::add(std::size_t delta) noexcept -> void
{
    count.fetch_add(delta, std::memory_order_relaxed);
}

auto uncat::exec::progress::set(std::size_t delta) noexcept -> void
{
    count.store(delta, std::memory_order_relaxed);
}

auto uncat::exec::progress::max(std::size_t delta) noexcept -> void
{
    total.store(delta, std::memory_order_relaxed);
}

//// control

auto uncat::exec::control::peek() const -> current
{
    return data->now();
}

auto uncat::exec::control::stat() const -> status
{
    return data->status();
}

auto uncat::exec::control::stop() -> void
{
    data->cancel();
}

auto uncat::exec::control::wait() const -> void
{
    data->holdon();
}

uncat::exec::control::control(sensor && ex)
    : data()
{
    if (ex.data)
    {
        ex.data->decrease();
        data = std::move(ex.data);
        ex.data = nullptr;
    }
}

//// sensor

auto uncat::exec::sensor::add(std::size_t i) const -> void
{
    data->add(i);
}

auto uncat::exec::sensor::set(std::size_t i) const -> void
{
    data->set(i);
}

auto uncat::exec::sensor::max(std::size_t i) const -> void
{
    data->max(i);
}

auto uncat::exec::sensor::expired() const -> bool
{
    return data->expire();
}

uncat::exec::sensor::~sensor()
{
    if (data)
        data->decrease();
}

uncat::exec::sensor::sensor(std::size_t max)
    : data(std::make_shared<progress>())
{
    data->increase();
    if (max != 0)
        data->max(max);
}

uncat::exec::sensor::sensor(sensor && rhs) noexcept
    : data(std::move(rhs.data))
{
    rhs.data = nullptr;
}

uncat::exec::sensor::sensor(sensor const & rhs)
    : data(rhs.data)
{
    if (data)
        data->increase();
}

uncat::exec::sensor & uncat::exec::sensor::operator=(sensor && rhs) noexcept
{
    if (data)
        data->decrease();
    data = std::move(rhs.data);
    rhs.data = nullptr;
    return *this;
}

uncat::exec::sensor & uncat::exec::sensor::operator=(sensor const & rhs)
{
    if (data)
        data->decrease();
    data = rhs.data;
    if (data)
        data->increase();
    return *this;
}
