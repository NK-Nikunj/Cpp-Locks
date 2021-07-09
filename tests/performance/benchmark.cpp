// Copyright (c) 2021 Nikunj Gupta

#include <locks.hpp>
#include <util/benchmark.hpp>

#include <hpx/chrono.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/async.hpp>
#include <hpx/modules/futures.hpp>
#include <hpx/modules/lcos_local.hpp>

#include <cstdint>
#include <string>
#include <tuple>

////////////////////////////////////////////////////////////////////////////////
template <typename LockType>
struct critical_cases
{
    void base_case(std::uint64_t grain_size)
    {
        // Do artificial work for grain_size
        hpx::chrono::high_resolution_timer t;
        while (t.elapsed() * 1e6 < grain_size)
        {
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Critical_small does absolutely minimum work in the critical section
    // making it adequate to compare with situation where atomicity is expected
    // from a minor code section.
    void critical_small(std::uint64_t grain_size)
    {
        // Do artificial work for grain_size
        hpx::chrono::high_resolution_timer t;
        while (t.elapsed() * 1e6 < grain_size)
        {
        }

        std::lock_guard<LockType> guard(lock);
        ++counter;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Critical_med does half the work in the critical section making it
    // adequate to compare with parallel graph algorithms where decent chunk of
    //  work is done under locked conditions.
    void critical_med(std::uint64_t grain_size)
    {
        // Do artificial work for grain_size/2
        hpx::chrono::high_resolution_timer t1;
        while (t1.elapsed() * 1e6 < (grain_size / 2))
        {
        }

        std::lock_guard<LockType> guard(lock);
        ++counter;
        // Do artificial work for 50us
        hpx::chrono::high_resolution_timer t2;
        while (t2.elapsed() * 1e6 < (grain_size / 2))
        {
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Critical_big does all of its work in the critical section making it
    // adequate to compare with lock-based queues/linked-lists where majority of
    //  the code is under locks.
    void critical_big(std::uint64_t grain_size)
    {
        std::lock_guard<LockType> guard(lock);
        ++counter;

        // Do artificial work for grain_size
        hpx::chrono::high_resolution_timer t;
        while (t.elapsed() * 1e6 < grain_size)
        {
        }
    }

private:
    std::uint64_t counter{};
    LockType lock{};
};
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void no_locks(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<hpx::lcos::local::spinlock> cases;

    for (std::uint64_t i = 0ul; i != num_tasks; ++i)
        futures.emplace_back(
            hpx::async(&critical_cases<hpx::lcos::local::spinlock>::base_case,
                &cases, grain_size));

    hpx::wait_all(futures);
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
template <typename LockType>
void critical_small(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<LockType> cases;

    for (std::uint64_t i = 0ul; i != num_tasks; ++i)
        futures.emplace_back(hpx::async(
            &critical_cases<LockType>::critical_small, &cases, grain_size));

    hpx::wait_all(futures);
}

template <typename LockType>
void critical_med(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<LockType> cases;

    for (std::uint64_t i = 0ul; i != num_tasks; ++i)
        futures.emplace_back(hpx::async(
            &critical_cases<LockType>::critical_med, &cases, grain_size));

    hpx::wait_all(futures);
}

template <typename LockType>
void critical_big(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<LockType> cases;

    for (std::uint64_t i = 0ul; i != num_tasks; ++i)
        futures.emplace_back(hpx::async(
            &critical_cases<LockType>::critical_big, &cases, grain_size));

    hpx::wait_all(futures);
}
////////////////////////////////////////////////////////////////////////////////

int hpx_main(hpx::program_options::variables_map& vm)
{
    std::uint64_t num_tasks = vm["num-tasks"].as<std::uint64_t>();
    std::uint64_t grain_size = vm["grain-size"].as<std::uint64_t>();

    locks::util::benchmark_invoker invoker{num_tasks, grain_size};
    invoker.invoke(GET_FUNCTION_PAIR(no_locks),
        GET_FUNCTION_PAIR(critical_small<hpx::lcos::local::spinlock>),
        GET_FUNCTION_PAIR(critical_med<hpx::lcos::local::spinlock>),
        GET_FUNCTION_PAIR(critical_big<hpx::lcos::local::spinlock>),
        GET_FUNCTION_PAIR(critical_small<locks::TAS_lock>),
        GET_FUNCTION_PAIR(critical_med<locks::TAS_lock>),
        GET_FUNCTION_PAIR(critical_big<locks::TAS_lock>),
        GET_FUNCTION_PAIR(critical_small<locks::TTAS_lock>),
        GET_FUNCTION_PAIR(critical_med<locks::TTAS_lock>),
        GET_FUNCTION_PAIR(critical_big<locks::TTAS_lock>),
        GET_FUNCTION_PAIR(critical_small<locks::MCS_lock>),
        GET_FUNCTION_PAIR(critical_med<locks::MCS_lock>),
        GET_FUNCTION_PAIR(critical_big<locks::MCS_lock>)
        // GET_FUNCTION_PAIR(critical_small<locks::CLH_lock>),
        // GET_FUNCTION_PAIR(critical_med<locks::CLH_lock>),
        // GET_FUNCTION_PAIR(critical_big<locks::CLH_lock>)
        //
    );

    return hpx::finalize();    // Handles HPX shutdown
}

int main(int argc, char* argv[])
{
    hpx::program_options::options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");

    desc_commandline.add_options()("num-tasks",
        hpx::program_options::value<std::uint64_t>()->default_value(10000),
        "Number of tasks to launch");
    desc_commandline.add_options()("grain-size",
        hpx::program_options::value<std::uint64_t>()->default_value(100),
        "Grain size of each task");

    // Initialize and run HPX
    hpx::init_params init_args;
    init_args.desc_cmdline = desc_commandline;

    return hpx::init(argc, argv, init_args);
}
