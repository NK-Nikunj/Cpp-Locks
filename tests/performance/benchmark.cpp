// Copyright (c) 2021 Nikunj Gupta

#include <locks.hpp>

#include <hpx/chrono.hpp>
#include <hpx/hpx_main.hpp>
#include <hpx/include/async.hpp>
#include <hpx/modules/futures.hpp>
#include <hpx/modules/lcos_local.hpp>

#include <benchmark/benchmark.h>

#include <cstdint>
#include <mutex>

////////////////////////////////////////////////////////////////////////////////
// Critical_small does absolutely minimum work in the critical section
// making it adequate to compare with situation where atomicity is expected
// from a minor code section.
void critical_small()
{
    std::uint64_t counter{};

    locks::TAS_lock lock;

    {
        std::lock_guard<locks::TAS_lock> guard(lock);
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
void critical_med()
{
    std::uint64_t counter{};

    locks::TAS_lock lock;

    {
        std::lock_guard<locks::TAS_lock> guard(lock);
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
void critical_big()
{
    std::uint64_t counter{};

    locks::TAS_lock lock;

    {
        std::lock_guard<locks::TAS_lock> guard(lock);
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
static void tas_critical_small(benchmark::State& state)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(1000000);

    for (auto _ : state)
    {
        for (std::uint64_t i = 0ul; i != 1000000ul; ++i)
            futures.emplace_back(hpx::async(critical_small));

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
            futures.emplace_back(hpx::async(critical_med));

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
            futures.emplace_back(hpx::async(critical_big));

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
            futures.emplace_back(hpx::async(critical_small));

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
            futures.emplace_back(hpx::async(critical_med));

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
            futures.emplace_back(hpx::async(critical_big));

        hpx::wait_all(futures);
    }
}
BENCHMARK(ttas_critical_big);
////////////////////////////////////////////////////////////////////////////////

BENCHMARK_MAIN();
