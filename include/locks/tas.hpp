// Copyright (c) 2021 Nikunj Gupta

#pragma once

#include <atomic>

namespace locks {

    class TAS_lock
    {
    public:
        TAS_lock() = default;
        TAS_lock(TAS_lock const&) = delete;
        TAS_lock(TAS_lock&&) = delete;

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
        }
    }

    inline void TAS_lock::unlock()
    {
        is_locked_.store(false);
    }

    inline bool TAS_lock::is_locked()
    {
        return is_locked_.load();
    }

}    // namespace locks