#include <locks.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/modules/algorithms.hpp>
#include <hpx/modules/lcos_local.hpp>

#include <mutex>
#include <queue>

auto return_bounded_function(
    std::function<void(std::uint64_t)> func, std::string const& name)
{
    return std::make_pair(func, name);
}
#define GET_FUNCTION_PAIR(f) return_bounded_function(f, #f)

namespace ds {

    template <typename ValueType, typename LockType>
    class Queue
    {
    public:
        void pop()
        {
            std::lock_guard<LockType> mlock(lock_);
            auto item = queue_.front();
            queue_.pop();
        }

        void push(const ValueType& item)
        {
            std::lock_guard<LockType> mlock(lock_);
            queue_.push(item);
        }

    private:
        std::queue<ValueType> queue_;
        LockType lock_;
    };

}    // namespace ds

////////////////////////////////////////////////////////////////////////////////
void hpx_spinlock(std::uint64_t num_push_pop)
{
    ds::Queue<int, hpx::lcos::local::spinlock> queue;

    hpx::for_loop(0ul, num_push_pop,
        [&queue](std::uint64_t i) { queue.push(std::rand()); });

    hpx::for_loop(
        0ul, num_push_pop, [&queue](std::uint64_t i) { queue.pop(); });
}

////////////////////////////////////////////////////////////////////////////////
void tas_lock(std::uint64_t num_push_pop)
{
    ds::Queue<int, locks::TAS_lock> queue;

    hpx::for_loop(0ul, num_push_pop,
        [&queue](std::uint64_t i) { queue.push(std::rand()); });

    // hpx::for_loop(
    //     0ul, num_push_pop, [&queue](std::uint64_t i) { queue.pop(); });
}

////////////////////////////////////////////////////////////////////////////////
void ttas_lock(std::uint64_t num_push_pop)
{
    ds::Queue<int, locks::TTAS_lock> queue;

    hpx::for_loop(0ul, num_push_pop,
        [&queue](std::uint64_t i) { queue.push(std::rand()); });

    hpx::for_loop(
        0ul, num_push_pop, [&queue](std::uint64_t i) { queue.pop(); });
}

////////////////////////////////////////////////////////////////////////////////
void mcs_lock(std::uint64_t num_push_pop)
{
    ds::Queue<int, locks::MCS_lock> queue;

    hpx::for_loop(0ul, num_push_pop,
        [&queue](std::uint64_t i) { queue.push(std::rand()); });

    hpx::for_loop(
        0ul, num_push_pop, [&queue](std::uint64_t i) { queue.pop(); });
}

////////////////////////////////////////////////////////////////////////////////
class benchmark_invoker
{
public:
    benchmark_invoker(std::uint64_t num_push_pop)
      : num_push_pop_(num_push_pop)
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
            func.first(num_push_pop_);
        }
        double elapsed = t.elapsed() / 3;

        std::cout << std::left << std::setw(30) << func.second << elapsed
                  << '\n';

        this->invoke(args...);
    }

private:
    std::uint64_t num_push_pop_;
};

int hpx_main(hpx::program_options::variables_map& vm)
{
    // extract command line argument, i.e. fib(N)
    std::uint64_t num_push_pop = vm["num-push-pop"].as<std::uint64_t>();

    benchmark_invoker invoker{num_push_pop};
    invoker.invoke(GET_FUNCTION_PAIR(hpx_spinlock),
        GET_FUNCTION_PAIR(ttas_lock), GET_FUNCTION_PAIR(mcs_lock),
        GET_FUNCTION_PAIR(tas_lock));

    return hpx::finalize();    // Handles HPX shutdown
}

int main(int argc, char* argv[])
{
    hpx::program_options::options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");

    desc_commandline.add_options()("num-push-pop",
        hpx::program_options::value<std::uint64_t>()->default_value(10000),
        "Number of Push-Pop operations");

    // Initialize and run HPX
    hpx::init_params init_args;
    init_args.desc_cmdline = desc_commandline;

    return hpx::init(argc, argv, init_args);
}
