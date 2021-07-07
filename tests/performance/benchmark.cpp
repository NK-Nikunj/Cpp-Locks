// Copyright (c) 2021 Nikunj Gupta

#include <locks.hpp>

#include <hpx/chrono.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/async.hpp>
#include <hpx/modules/futures.hpp>
#include <hpx/modules/lcos_local.hpp>

#include <cstdint>
#include <string>

std::string return_function_name(
    std::function<void(std::uint64_t, std::uint64_t)> func,
    std::string const& name)
{
    return name;
}
#define GET_FUNCTION_NAME(f) return_function_name(f, #f)

////////////////////////////////////////////////////////////////////////////////
void base_case(std::uint64_t grain_size)
{
    // Do artificial work for grain_size
    hpx::chrono::high_resolution_timer t;
    while (t.elapsed() * 1e6 < grain_size)
    {
    }
}

////////////////////////////////////////////////////////////////////////////////
// Critical_small does absolutely minimum work in the critical section
// making it adequate to compare with situation where atomicity is expected
// from a minor code section.
template <typename LockType>
void critical_small(std::uint64_t grain_size)
{
    std::uint64_t counter{};

    LockType lock{};

    {
        std::lock_guard<LockType> guard(lock);
        ++counter;
    }

    // Do artificial work for grain_size
    hpx::chrono::high_resolution_timer t;
    while (t.elapsed() * 1e6 < grain_size)
    {
    }
}

////////////////////////////////////////////////////////////////////////////////
// Critical_med does half the work in the critical section making it adequate to
// compare with parallel graph algorithms where decent chunk of work is done
// under locked conditions.
template <typename LockType>
void critical_med(std::uint64_t grain_size)
{
    std::uint64_t counter{};

    LockType lock{};

    {
        std::lock_guard<LockType> guard(lock);
        ++counter;
        // Do artificial work for 50us
        hpx::chrono::high_resolution_timer t;
        while (t.elapsed() * 1e6 < (grain_size / 2))
        {
        }
    }

    // Do artificial work for grain_size/2
    hpx::chrono::high_resolution_timer t;
    while (t.elapsed() * 1e6 < (grain_size / 2))
    {
    }
}

////////////////////////////////////////////////////////////////////////////////
// Critical_big does all of its work in the critical section making it adequate
// to compare with lock-based queues/linked-lists where majority of the code
// is under locks.
template <typename LockType>
void critical_big(std::uint64_t grain_size)
{
    std::uint64_t counter{};

    LockType lock;

    {
        std::lock_guard<LockType> guard(lock);
        ++counter;
    }

    // Do artificial work for grain_size
    hpx::chrono::high_resolution_timer t;
    while (t.elapsed() * 1e6 < grain_size)
    {
    }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void no_locks(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(hpx::async(base_case, grain_size));

        hpx::wait_all(futures);
    }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void hpx_spinlock_small(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(hpx::async(
                critical_small<hpx::lcos::local::spinlock>, grain_size));

        hpx::wait_all(futures);
    }
}

void hpx_spinlock_med(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(hpx::async(
                critical_med<hpx::lcos::local::spinlock>, grain_size));

        hpx::wait_all(futures);
    }
}

void hpx_spinlock_big(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(hpx::async(
                critical_big<hpx::lcos::local::spinlock>, grain_size));

        hpx::wait_all(futures);
    }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void tas_critical_small(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(critical_small<locks::TAS_lock>, grain_size));

        hpx::wait_all(futures);
    }
}

void tas_critical_med(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(critical_med<locks::TAS_lock>, grain_size));

        hpx::wait_all(futures);
    }
}

void tas_critical_big(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(critical_big<locks::TAS_lock>, grain_size));

        hpx::wait_all(futures);
    }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void ttas_critical_small(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(critical_small<locks::TTAS_lock>, grain_size));

        hpx::wait_all(futures);
    }
}

void ttas_critical_med(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(critical_med<locks::TTAS_lock>, grain_size));

        hpx::wait_all(futures);
    }
}

void ttas_critical_big(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(critical_big<locks::TTAS_lock>, grain_size));

        hpx::wait_all(futures);
    }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void mcs_critical_small(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(critical_small<locks::MCS_lock>, grain_size));

        hpx::wait_all(futures);
    }
}

void mcs_critical_med(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(critical_med<locks::MCS_lock>, grain_size));

        hpx::wait_all(futures);
    }
}

void mcs_critical_big(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(critical_big<locks::MCS_lock>, grain_size));

        hpx::wait_all(futures);
    }
}
////////////////////////////////////////////////////////////////////////////////

class benchmark_invoker
{
public:
    benchmark_invoker(std::uint64_t num_tasks, std::uint64_t grain_size)
      : num_tasks_(num_tasks)
      , grain_size_(grain_size)
    {
    }

    template <typename Func, typename... Args>
    void invoke(Func&& func, Args&&... args)
    {
        hpx::chrono::high_resolution_timer t;
        for (std::size_t i = 0u; i != 3; ++i)
        {
            func(num_tasks_, grain_size_);
        }
        double elapsed = t.elapsed();

        std::string func_name = GET_FUNCTION_NAME(func);
        char const* fmt = "{1}\t\t{2}";
        hpx::util::format_to(std::cout, fmt, func_name, elapsed);
    }

private:
    std::uint64_t num_tasks_;
    std::uint64_t grain_size_;
};

int hpx_main(hpx::program_options::variables_map& vm)
{
    // extract command line argument, i.e. fib(N)
    std::uint64_t num_tasks = vm["num-tasks"].as<std::uint64_t>();
    std::uint64_t grain_size = vm["grain-size"].as<std::uint64_t>();

    benchmark_invoker invoker{num_tasks, grain_size};
    invoker.invoke(no_locks, hpx_spinlock_small, hpx_spinlock_med,
        hpx_spinlock_big, tas_critical_small, tas_critical_med,
        tas_critical_big, ttas_critical_small, ttas_critical_med,
        ttas_critical_big, mcs_critical_small, mcs_critical_med,
        mcs_critical_big);

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
