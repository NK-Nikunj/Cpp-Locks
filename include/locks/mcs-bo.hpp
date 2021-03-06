// Copyright (c) 2021 Nikunj Gupta

#pragma once

#include <util/backoff.hpp>

#include <hpx/config.hpp>
#include <hpx/modules/lcos_local.hpp>
#include <hpx/modules/threading.hpp>

#include <atomic>
#include <cstdint>

namespace locks {

    class MCS_BO_lock
    {
    private:
        struct mcs_node
        {
            std::uint32_t locked{false};
            mcs_node* next{nullptr};
        };

    public:
        MCS_BO_lock() = default;
        HPX_NON_COPYABLE(MCS_BO_lock);

        ~MCS_BO_lock() = default;

        void lock();
        void unlock();

    private:
        std::atomic<mcs_node*> tail{nullptr};
    };

    inline void MCS_BO_lock::lock()
    {
        mcs_node* local_node = new mcs_node{};
        hpx::threads::thread_id_type id = hpx::threads::get_self_id();
        hpx::threads::set_thread_data(
            id, reinterpret_cast<std::size_t>(local_node));

        mcs_node* const prev_node =
            tail.exchange(local_node, std::memory_order_acquire);

        if (prev_node != nullptr)
        {
            local_node->locked = true;

            prev_node->next = local_node;

            hpx::util::yield_while([local_node] { return local_node->locked; },
                "locks::MCS_BO_lock::lock");
        }
    }

    inline void MCS_BO_lock::unlock()
    {
        hpx::threads::thread_id_type id = hpx::threads::get_self_id();
        mcs_node* const curr_node =
            reinterpret_cast<mcs_node*>(hpx::threads::get_thread_data(id));

        if (curr_node->next == nullptr)
        {
            mcs_node* p = curr_node;
            if (tail.compare_exchange_strong(p, nullptr,
                    std::memory_order_release, std::memory_order_relaxed))
                return;

            while (curr_node->next == nullptr)
                HPX_SMT_PAUSE;
        }

        curr_node->next->locked = false;
        curr_node->next = nullptr;

        delete curr_node;
    }

}    // namespace locks
