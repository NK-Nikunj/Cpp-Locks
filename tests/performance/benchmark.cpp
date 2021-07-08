// Copyright (c) 2021 Nikunj Gupta

#include <locks.hpp>

#include <hpx/chrono.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/async.hpp>
#include <hpx/modules/futures.hpp>
#include <hpx/modules/lcos_local.hpp>

#include <cstdint>
#include <string>
#include <tuple>

auto return_bounded_function(
    std::function<void(std::uint64_t, std::uint64_t)> func,
    std::string const& name)
{
    return std::make_pair(func, name);
}
#define GET_FUNCTION_PAIR(f) return_bounded_function(f, #f)

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

        {
            std::lock_guard<LockType> guard(lock);
            ++counter;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Critical_med does half the work in the critical section making it
    // adequate to compare with parallel graph algorithms where decent chunk of
    //  work is done under locked conditions.
    void critical_med(std::uint64_t grain_size)
    {
        // Do artificial work for grain_size/2
        hpx::chrono::high_resolution_timer t;
        while (t.elapsed() * 1e6 < (grain_size / 2))
        {
        }

        {
            std::lock_guard<LockType> guard(lock);
            ++counter;
            // Do artificial work for 50us
            hpx::chrono::high_resolution_timer t;
            while (t.elapsed() * 1e6 < (grain_size / 2))
            {
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Critical_big does all of its work in the critical section making it
    // adequate to compare with lock-based queues/linked-lists where majority of
    //  the code is under locks.
    void critical_big(std::uint64_t grain_size)
    {
        {
            std::lock_guard<LockType> guard(lock);
            ++counter;

            // Do artificial work for grain_size
            hpx::chrono::high_resolution_timer t;
            while (t.elapsed() * 1e6 < grain_size)
            {
            }
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

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(hpx::async(
                &critical_cases<hpx::lcos::local::spinlock>::base_case, &cases,
                grain_size));

        hpx::wait_all(futures);
    }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void hpx_spinlock_small(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<hpx::lcos::local::spinlock> cases;

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(hpx::async(
                &critical_cases<hpx::lcos::local::spinlock>::critical_small,
                &cases, grain_size));

        hpx::wait_all(futures);
    }
}

void hpx_spinlock_med(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<hpx::lcos::local::spinlock> cases;

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(hpx::async(
                &critical_cases<hpx::lcos::local::spinlock>::critical_med,
                &cases, grain_size));

        hpx::wait_all(futures);
    }
}

void hpx_spinlock_big(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<hpx::lcos::local::spinlock> cases;

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(hpx::async(
                &critical_cases<hpx::lcos::local::spinlock>::critical_big,
                &cases, grain_size));

        hpx::wait_all(futures);
    }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void tas_critical_small(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<locks::TAS_lock> cases;

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(&critical_cases<locks::TAS_lock>::critical_small,
                    &cases, grain_size));

        hpx::wait_all(futures);
    }
}

void tas_critical_med(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<locks::TAS_lock> cases;

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(&critical_cases<locks::TAS_lock>::critical_med,
                    &cases, grain_size));

        hpx::wait_all(futures);
    }
}

void tas_critical_big(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<locks::TAS_lock> cases;

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(&critical_cases<locks::TAS_lock>::critical_big,
                    &cases, grain_size));

        hpx::wait_all(futures);
    }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void ttas_critical_small(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<locks::TTAS_lock> cases;

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(&critical_cases<locks::TTAS_lock>::critical_small,
                    &cases, grain_size));

        hpx::wait_all(futures);
    }
}

void ttas_critical_med(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<locks::TTAS_lock> cases;

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(&critical_cases<locks::TTAS_lock>::critical_med,
                    &cases, grain_size));

        hpx::wait_all(futures);
    }
}

void ttas_critical_big(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<locks::TTAS_lock> cases;

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(&critical_cases<locks::TTAS_lock>::critical_big,
                    &cases, grain_size));

        hpx::wait_all(futures);
    }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void mcs_critical_small(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<locks::MCS_lock> cases;

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(&critical_cases<locks::MCS_lock>::critical_small,
                    &cases, grain_size));

        hpx::wait_all(futures);
    }
}

void mcs_critical_med(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<locks::MCS_lock> cases;

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(&critical_cases<locks::MCS_lock>::critical_med,
                    &cases, grain_size));

        hpx::wait_all(futures);
    }
}

void mcs_critical_big(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    std::vector<hpx::future<void>> futures;
    futures.reserve(num_tasks);

    critical_cases<locks::MCS_lock> cases;

    {
        for (std::uint64_t i = 0ul; i != num_tasks; ++i)
            futures.emplace_back(
                hpx::async(&critical_cases<locks::MCS_lock>::critical_big,
                    &cases, grain_size));

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
        std::cout << std::left << std::setw(30) << "Name: "
                  << "Time (in s)" << '\n';
    }

    void invoke()
    {
        std::cout << std::left << std::setw(30) << "Name: "
                  << "Time (in s)" << '\n';
    }

    template <typename Func, typename... Args>
    void invoke(Func&& func, Args&&... args)
    {
        hpx::chrono::high_resolution_timer t;
        for (std::size_t i = 0u; i != 3; ++i)
        {
            func.first(num_tasks_, grain_size_);
        }
        double elapsed = t.elapsed() / 3;

        std::cout << std::left << std::setw(30) << func.second << elapsed
                  << '\n';

        this->invoke(args...);
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
    invoker.invoke(GET_FUNCTION_PAIR(no_locks),
        GET_FUNCTION_PAIR(hpx_spinlock_small),
        GET_FUNCTION_PAIR(hpx_spinlock_med),
        GET_FUNCTION_PAIR(hpx_spinlock_big),
        GET_FUNCTION_PAIR(tas_critical_small),
        GET_FUNCTION_PAIR(tas_critical_med),
        GET_FUNCTION_PAIR(tas_critical_big),
        GET_FUNCTION_PAIR(ttas_critical_small),
        GET_FUNCTION_PAIR(ttas_critical_med),
        GET_FUNCTION_PAIR(ttas_critical_big),
        GET_FUNCTION_PAIR(mcs_critical_small),
        GET_FUNCTION_PAIR(mcs_critical_med),
        GET_FUNCTION_PAIR(mcs_critical_big));

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
