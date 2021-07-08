// Copyright (c) 2021 Nikunj Gupta

#pragma once

#include <hpx/config.hpp>

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
        HPX_NON_COPYABLE(MCS_lock);

        void lock();
        void unlock();

    private:
        std::atomic<mcs_node*> tail{nullptr};
        static thread_local mcs_node local_node;
    };

    void MCS_lock::lock()
    {
        const auto prev_node = tail.exchange(&local_node);

        if (prev_node != nullptr)
        {
            local_node.locked = true;

            prev_node->next = &local_node;

            while (local_node.locked)
            {
                asm volatile("pause\n" : : : "memory");
            }
        }
    }

    void MCS_lock::unlock()
    {
        if (local_node.next == nullptr)
        {
            mcs_node* p = &local_node;
            if (tail.compare_exchange_strong(p, nullptr))
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
