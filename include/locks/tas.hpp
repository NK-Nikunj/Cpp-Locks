// Copyright (c) 2021 Nikunj Gupta

#pragma once

#include <hpx/config.hpp>

#include <immintrin.h>

#include <atomic>
#include <iostream>

namespace locks {

    class TAS_lock
    {
    public:
        TAS_lock() = default;
        HPX_NON_COPYABLE(TAS_lock);

        void lock();
        void unlock();
        bool is_locked();

    private:
        std::atomic<bool> is_locked_;
    };

    inline void TAS_lock::lock()
    {
        while (is_locked_.exchange(true, std::memory_order_acquire))
        {
            asm volatile("pause\n" : : : "memory");
        }
    }

    inline void TAS_lock::unlock()
    {
        is_locked_.store(false, std::memory_order_release);
    }

    inline bool TAS_lock::is_locked()
    {
        return is_locked_.load(std::memory_order_acquire);
    }

}    // namespace locks
