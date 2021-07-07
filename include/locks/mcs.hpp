// Copyright (c) 2021 Nikunj Gupta

#pragma once

#include <atomic>
#include <cstdint>

namespace locks {

    class MCS_lock
    {
    private:
        struct mcs_node
        {
            bool locked{false};
            mcs_node* next{nullptr};
        };

    public:
        MCS_lock() = default;
        MCS_lock(MCS_lock const&) = delete;
        MCS_lock(MCS_lock&&) = delete;

        void lock();
        void unlock();

    private:
        std::atomic<mcs_node*> tail{nullptr};
        static thread_local mcs_node local_node;
    };

    void MCS_lock::lock()
    {
        const auto prev_node =
            tail.exchange(&local_node, std::memory_order_acquire);

        if (prev_node != nullptr)
        {
            local_node.locked = true;

            prev_node->next = &local_node;

            while (local_node.locked)
            {
            }
        }
    }

    void MCS_lock::unlock()
    {
        if (local_node.next == nullptr)
        {
            mcs_node* p = &local_node;
            if (tail.compare_exchange_strong(p, nullptr,
                    std::memory_order_release, std::memory_order_relaxed))
                return;

            while (local_node.next == nullptr)
            {
            }

            local_node.next->locked = false;
            local_node.next = nullptr;
        }
    }

    thread_local MCS_lock::mcs_node MCS_lock::local_node{};

}    // namespace locks
