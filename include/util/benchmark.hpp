// Copyright (c) 2021 Nikunj Gupta

#pragma once

#include <hpx/chrono.hpp>
#include <hpx/include/util.hpp>

#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>

namespace locks { namespace util {

    template <typename Func>
    auto return_bounded_function(Func&& func, std::string const& name)
    {
        return std::make_pair(func, name);
    }

    template <typename... Tuple>
    class benchmark_invoker
    {
    public:
        benchmark_invoker(Tuple... args)
          : arg_list(args...)
        {
            std::cout << std::left << std::setw(50) << "Name: "
                      << "Time (in s)" << '\n';
        }

        void invoke()
        {
            std::cout << std::left << std::setw(50) << "Name: "
                      << "Time (in s)" << '\n';
        }

        template <typename Func, typename... Args>
        void invoke(Func&& func, Args&&... args)
        {
            hpx::chrono::high_resolution_timer t;
            for (std::size_t i = 0u; i != 3; ++i)
            {
                hpx::util::invoke_fused(func.first, arg_list);
            }
            double elapsed = t.elapsed() / 3;

            std::cout << std::left << std::setw(50) << func.second << elapsed
                      << '\n';

            this->invoke(args...);
        }

    private:
        std::tuple<Tuple...> arg_list;
    };
}}    // namespace locks::util

#define GET_FUNCTION_PAIR(f) locks::util::return_bounded_function(f, #f)
