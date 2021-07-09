// Copyright (c) 2021 Nikunj Gupta

#pragma once

#include <hpx/config.hpp>
#include <hpx/modules/lcos_local.hpp>
#include <hpx/modules/threading.hpp>

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
    };

    void MCS_lock::lock()
    {
        mcs_node* const local_node = new mcs_node();

        hpx::threads::thread_id_type id = hpx::threads::get_self_id();
        hpx::threads::set_thread_data(
            id, reinterpret_cast<std::size_t>(local_node));

        mcs_node* const prev_node =
            tail.exchange(local_node, std::memory_order_acquire);

        if (prev_node != nullptr)
        {
            local_node->locked = true;

            prev_node->next = local_node;

            while (local_node->locked)
            {
            }
        }
    }

    void MCS_lock::unlock()
    {
        // Node 1 gets access to its local node
        hpx::threads::thread_id_type id = hpx::threads::get_self_id();
        mcs_node* const prev_node =
            reinterpret_cast<mcs_node*>(hpx::threads::get_thread_data(id));

        if (prev_node->next == nullptr)
        {
            mcs_node* p = prev_node;
            if (tail.compare_exchange_strong(p, nullptr,
                    std::memory_order_release, std::memory_order_relaxed))
                return;

            while (prev_node->next == nullptr)
            {
            }
        }

        prev_node->next->locked = false;
        prev_node->next = nullptr;
    }

}    // namespace locks
