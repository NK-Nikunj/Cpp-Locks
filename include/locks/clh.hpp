// Copyright (c) 2021 Nikunj Gupta

#pragma once

#include <hpx/config.hpp>
#include <hpx/modules/lcos_local.hpp>
#include <hpx/modules/threading.hpp>

#include <atomic>
#include <cstdint>

namespace locks {

    class CLH_lock
    {
    private:
        struct clh_node
        {
            std::uint64_t locked{false};
        };

    public:
        CLH_lock() = default;
        HPX_NON_COPYABLE(CLH_lock);

        ~CLH_lock()
        {
            delete tail;
        }

        void lock();
        void unlock();

    private:
        std::atomic<clh_node*> tail{new clh_node()};
    };

    inline void CLH_lock::lock()
    {
        std::unique_ptr<clh_node> local_node = std::make_unique<clh_node>();

        hpx::threads::thread_id_type id = hpx::threads::get_self_id();
        hpx::threads::set_thread_data(
            id, reinterpret_cast<std::size_t>(&local_node));

        local_node->locked = true;

        clh_node* const prev_node =
            tail.exchange(local_node.get(), std::memory_order_acquire);

        while (prev_node->locked)
        {
        }
    }

    inline void CLH_lock::unlock()
    {
        hpx::threads::thread_id_type id = hpx::threads::get_self_id();
        clh_node* const prev_node =
            reinterpret_cast<clh_node*>(hpx::threads::get_thread_data(id));

        prev_node->locked = false;
    }

}    // namespace locks
