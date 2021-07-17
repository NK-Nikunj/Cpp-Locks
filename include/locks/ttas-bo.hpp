// Copyright (c) 2021 Nikunj Gupta

#pragma once

#include <util/backoff.hpp>

#include <hpx/config.hpp>

#include <atomic>

namespace locks {

    class TTAS_BO_lock
    {
    public:
        TTAS_BO_lock() = default;
        HPX_NON_COPYABLE(TTAS_BO_lock);

        void lock();
        void unlock();
        bool is_locked();

    private:
        std::atomic<bool> is_locked_;
    };

    inline void TTAS_BO_lock::lock()
    {
        do
        {
            hpx::util::yield_while(
                [this] { return is_locked(); }, "locks::TTAS_BO_lock::lock");
        } while (is_locked_.exchange(true, std::memory_order_acquire));
    }

    inline void TTAS_BO_lock::unlock()
    {
        is_locked_.store(false, std::memory_order_release);
    }

    inline bool TTAS_BO_lock::is_locked()
    {
        return is_locked_.load(std::memory_order_relaxed);
    }

}    // namespace locks
