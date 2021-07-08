#include <locks.hpp>
#include <util/benchmark.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/modules/algorithms.hpp>
#include <hpx/modules/lcos_local.hpp>

#include <mutex>
#include <queue>

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
template <typename LockType>
void concurrent_queue(std::uint64_t num_push_pop)
{
    ds::Queue<std::uint64_t, LockType> queue;

    hpx::for_loop(0ul, num_push_pop,
        [&queue](std::uint64_t i) { queue.push(std::rand()); });

    hpx::for_loop(
        0ul, num_push_pop, [&queue](std::uint64_t i) { queue.pop(); });
}

int hpx_main(hpx::program_options::variables_map& vm)
{
    // extract command line argument, i.e. fib(N)
    std::uint64_t num_push_pop = vm["num-push-pop"].as<std::uint64_t>();

    locks::util::benchmark_invoker invoker{num_push_pop};
    invoker.invoke(
        GET_FUNCTION_PAIR(concurrent_queue<hpx::lcos::local::spinlock>),
        GET_FUNCTION_PAIR(concurrent_queue<locks::TAS_lock>),
        GET_FUNCTION_PAIR(concurrent_queue<locks::TTAS_lock>),
        GET_FUNCTION_PAIR(concurrent_queue<locks::MCS_lock>));

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
