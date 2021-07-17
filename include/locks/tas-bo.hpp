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
        bool acquire_lock();
        std::atomic<bool> is_locked_;
    };

    inline bool TAS_BO_lock::acquire_lock()
    {
        return !is_locked_.exchange(true, std::memory_order_acquire);
    }

    inline void TAS_BO_lock::lock()
    {
        hpx::util::yield_while(
            [this] { return !acquire_lock(); }, "locks::TAS_BO_lock::lock");
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
