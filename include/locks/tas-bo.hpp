// Copyright (c) 2021 Nikunj Gupta

#pragma once

#include <util/backoff.hpp>

#include <hpx/config.hpp>

#include <immintrin.h>

#include <atomic>
#include <iostream>

namespace locks {

    class TAS_BO_lock
    {
    public:
        TAS_BO_lock() = default;
        HPX_NON_COPYABLE(TAS_BO_lock);

        void lock();
        void unlock();
        bool is_locked();

    private:
        std::atomic<bool> is_locked_;
    };

    inline void TAS_BO_lock::lock()
    {
        std::size_t k = 0x1;
        while (is_locked_.exchange(true, std::memory_order_acquire))
        {
            k <<= 1;
            locks::util::exp_backoff(k);
        }
    }

    inline void TAS_BO_lock::unlock()
    {
        is_locked_.store(false, std::memory_order_release);
    }

    inline bool TAS_BO_lock::is_locked()
    {
        return is_locked_.load(std::memory_order_acquire);
    }

}    // namespace locks
