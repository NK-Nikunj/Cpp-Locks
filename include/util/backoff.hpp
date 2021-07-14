// Copyright (c) 2021 Nikunj Gupta

#pragma once

#include <hpx/config.hpp>
#include <hpx/modules/threading.hpp>

#include <cstdint>

namespace locks { namespace util {

    inline void exp_backoff(std::size_t k)
    {
        if (k > 32)
            hpx::this_thread::suspend();
        else if (k > 16)
            while (k >>= 1)
                HPX_SMT_PAUSE;
        else
            while (k >>= 1)
                ;
    }

}}    // namespace locks::util
