// Copyright (c) 2021 Nikunj Gupta

#include <locks.hpp>

#include <hpx/chrono.hpp>
#include <hpx/hpx_main.hpp>
#include <hpx/include/async.hpp>
#include <hpx/modules/futures.hpp>
#include <hpx/modules/lcos_local.hpp>

#include <benchmark/benchmark.h>

#include <cstdint>

////////////////////////////////////////////////////////////////////////////////
void base_case()
{
    // Do artificial work for 100us
    hpx::chrono::high_resolution_timer t;
    while (t.elapsed() * 1e6 < 100)
    {
    }
}

////////////////////////////////////////////////////////////////////////////////
// Critical_small does absolutely minimum work in the critical section
// making it adequate to compare with situation where atomicity is expected
// from a minor code section.
template <typename LockType>
void critical_small()
{
    std::uint64_t counter{};

    LockType lock{};

    {
        std::lock_guard<LockType> guard(lock);
        ++counter;
    }

    // Do artificial work for 100us
    hpx::chrono::high_resolution_timer t;
    while (t.elapsed() * 1e6 < 100)
    {
    }
}

////////////////////////////////////////////////////////////////////////////////
// Critical_med does half the work in the critical section making it adequate to
// compare with parallel graph algorithms where decent chunk of work is done
// under locked conditions.
template <typename LockType>
void critical_med()
{
    std::uint64_t counter{};

    LockType lock{};

    {
        std::lock_guard<LockType> guard(lock);
        ++counter;
        // Do artificial work for 50us
        hpx::chrono::high_resolution_timer t;
        while (t.elapsed() * 1e6 < 50)
        {
        }
    }

    // Do artificial work for 50us
    hpx::chrono::high_resolution_timer t;
    while (t.elapsed() * 1e6 < 50)
    {
    }
}

////////////////////////////////////////////////////////////////////////////////
// Critical_big does all of its work in the critical section making it adequate
// to compare with lock-based queues/linked-lists where majority of the code
// is under locks.
template <typename LockType>
void critical_big()
{
    std::uint64_t counter{};

    LockType lock;

    {
        std::lock_guard<LockType> guard(lock);
        ++counter;
    }

    // Do artificial work for 100us
    hpx::chrono::high_resolution_timer t;
    while (t.elapsed() * 1e6 < 100)
    {
    }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
static void no_locks(benchmark::State& state)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(1000000);

    for (auto _ : state)
    {
        for (std::uint64_t i = 0ul; i != 1000000ul; ++i)
            futures.emplace_back(hpx::async(base_case));

        hpx::wait_all(futures);
    }
}
BENCHMARK(no_locks);
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
static void hpx_spinlock_small(benchmark::State& state)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(1000000);

    for (auto _ : state)
    {
        for (std::uint64_t i = 0ul; i != 1000000ul; ++i)
            futures.emplace_back(
                hpx::async(critical_small<hpx::lcos::local::spinlock>));

        hpx::wait_all(futures);
    }
}
BENCHMARK(hpx_spinlock_small);

static void hpx_spinlock_med(benchmark::State& state)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(1000000);

    for (auto _ : state)
    {
        for (std::uint64_t i = 0ul; i != 1000000ul; ++i)
            futures.emplace_back(
                hpx::async(critical_med<hpx::lcos::local::spinlock>));

        hpx::wait_all(futures);
    }
}
BENCHMARK(hpx_spinlock_med);

static void hpx_spinlock_big(benchmark::State& state)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(1000000);

    for (auto _ : state)
    {
        for (std::uint64_t i = 0ul; i != 1000000ul; ++i)
            futures.emplace_back(
                hpx::async(critical_big<hpx::lcos::local::spinlock>));

        hpx::wait_all(futures);
    }
}
BENCHMARK(hpx_spinlock_big);
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
static void tas_critical_small(benchmark::State& state)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(1000000);

    for (auto _ : state)
    {
        for (std::uint64_t i = 0ul; i != 1000000ul; ++i)
            futures.emplace_back(hpx::async(critical_small<locks::TAS_lock>));

        hpx::wait_all(futures);
    }
}
BENCHMARK(tas_critical_small);

static void tas_critical_med(benchmark::State& state)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(1000000);

    for (auto _ : state)
    {
        for (std::uint64_t i = 0ul; i != 1000000ul; ++i)
            futures.emplace_back(hpx::async(critical_med<locks::TAS_lock>));

        hpx::wait_all(futures);
    }
}
BENCHMARK(tas_critical_med);

static void tas_critical_big(benchmark::State& state)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(1000000);

    for (auto _ : state)
    {
        for (std::uint64_t i = 0ul; i != 1000000ul; ++i)
            futures.emplace_back(hpx::async(critical_big<locks::TAS_lock>));

        hpx::wait_all(futures);
    }
}
BENCHMARK(tas_critical_big);
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
static void ttas_critical_small(benchmark::State& state)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(1000000);

    for (auto _ : state)
    {
        for (std::uint64_t i = 0ul; i != 1000000ul; ++i)
            futures.emplace_back(hpx::async(critical_small<locks::TTAS_lock>));

        hpx::wait_all(futures);
    }
}
BENCHMARK(ttas_critical_small);

static void ttas_critical_med(benchmark::State& state)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(1000000);

    for (auto _ : state)
    {
        for (std::uint64_t i = 0ul; i != 1000000ul; ++i)
            futures.emplace_back(hpx::async(critical_med<locks::TTAS_lock>));

        hpx::wait_all(futures);
    }
}
BENCHMARK(ttas_critical_med);

static void ttas_critical_big(benchmark::State& state)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(1000000);

    for (auto _ : state)
    {
        for (std::uint64_t i = 0ul; i != 1000000ul; ++i)
            futures.emplace_back(hpx::async(critical_big<locks::TTAS_lock>));

        hpx::wait_all(futures);
    }
}
BENCHMARK(ttas_critical_big);
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
static void mcs_critical_small(benchmark::State& state)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(1000000);

    for (auto _ : state)
    {
        for (std::uint64_t i = 0ul; i != 1000000ul; ++i)
            futures.emplace_back(hpx::async(critical_small<locks::MCS_lock>));

        hpx::wait_all(futures);
    }
}
BENCHMARK(mcs_critical_small);

static void mcs_critical_med(benchmark::State& state)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(1000000);

    for (auto _ : state)
    {
        for (std::uint64_t i = 0ul; i != 1000000ul; ++i)
            futures.emplace_back(hpx::async(critical_med<locks::MCS_lock>));

        hpx::wait_all(futures);
    }
}
BENCHMARK(mcs_critical_med);

static void mcs_critical_big(benchmark::State& state)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(1000000);

    for (auto _ : state)
    {
        for (std::uint64_t i = 0ul; i != 1000000ul; ++i)
            futures.emplace_back(hpx::async(critical_big<locks::MCS_lock>));

        hpx::wait_all(futures);
    }
}
BENCHMARK(mcs_critical_big);
////////////////////////////////////////////////////////////////////////////////

BENCHMARK_MAIN();
