// Copyright (c) 2021 Nikunj Gupta

#pragma once

#include <atomic>

namespace locks {

    class TTAS_lock
    {
    public:
        TTAS_lock() = default;
        TTAS_lock(TTAS_lock const&) = delete;
        TTAS_lock(TTAS_lock&&) = delete;

        void lock();
        void unlock();
        bool is_locked();

    private:
        std::atomic<bool> is_locked_;
    };

    inline void TTAS_lock::lock()
    {
        while (!is_locked_.load(std::memory_order_relaxed) &&
            is_locked_.exchange(true, std::memory_order_acquire))
        {
        }
    }

    inline void TTAS_lock::unlock()
    {
        is_locked_.store(false);
    }

    inline bool TTAS_lock::is_locked()
    {
        return is_locked_.load();
    }

}    // namespace locks