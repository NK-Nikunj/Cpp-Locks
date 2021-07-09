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
            std::uint32_t locked{false};
            mcs_node* next{nullptr};
        };

    public:
        MCS_lock() = default;
        HPX_NON_COPYABLE(MCS_lock);

        ~MCS_lock()
        {
            delete tail;
        }

        void lock();
        void unlock();

    private:
        std::atomic<mcs_node*> tail{nullptr};
    };

    void MCS_lock::lock()
    {
        std::unique_ptr<mcs_node> local_node = std::make_unique<mcs_node>();

        hpx::threads::thread_id_type id = hpx::threads::get_self_id();
        hpx::threads::set_thread_data(
            id, reinterpret_cast<std::size_t>(local_node.get()));

        mcs_node* const prev_node =
            tail.exchange(local_node.get(), std::memory_order_acquire);

        if (prev_node != nullptr)
        {
            local_node->locked = true;

            prev_node->next = local_node.get();

            while (local_node->locked)
            {
                HPX_SMT_PAUSE;
            }
        }
    }

    void MCS_lock::unlock()
    {
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
                HPX_SMT_PAUSE;
            }
        }

        prev_node->next->locked = false;
        prev_node->next = nullptr;
    }

}    // namespace locks
