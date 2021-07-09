// Copyright (c) 2021 Nikunj Gupta

#pragma once

#include <hpx/config.hpp>

#include <atomic>

namespace locks {

    class TTAS_lock
    {
    public:
        TTAS_lock() = default;
        HPX_NON_COPYABLE(TTAS_lock);

        void lock();
        void unlock();
        bool is_locked();

    private:
        std::atomic<bool> is_locked_;
    };

    inline void TTAS_lock::lock()
    {
        while (true)
        {
            if (!is_locked_.load(std::memory_order_acquire) &&
                !is_locked_.exchange(true, std::memory_order_acquire))
                return;

            HPX_SMT_PAUSE;
        }
    }

    inline void TTAS_lock::unlock()
    {
        is_locked_.store(false, std::memory_order_release);
    }

    inline bool TTAS_lock::is_locked()
    {
        return is_locked_.load(std::memory_order_acquire);
    }

}    // namespace locks
