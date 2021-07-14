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
            clh_node() = default;
            clh_node(std::uint64_t value)
              : locked(value)
            {
            }

            std::uint64_t locked{true};
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
        std::atomic<clh_node*> tail{new clh_node(false)};
    };

    inline void CLH_lock::lock()
    {
        clh_node* local_node = new clh_node{};

        hpx::threads::thread_id_type id = hpx::threads::get_self_id();
        hpx::threads::set_thread_data(
            id, reinterpret_cast<std::size_t>(local_node));

        clh_node* const prev_node =
            tail.exchange(local_node, std::memory_order_acquire);

        while (prev_node->locked)
        {
        }

        delete prev_node;
    }

    inline void CLH_lock::unlock()
    {
        hpx::threads::thread_id_type id = hpx::threads::get_self_id();
        clh_node* const curr_node =
            reinterpret_cast<clh_node*>(hpx::threads::get_thread_data(id));

        curr_node->locked = false;
    }

}    // namespace locks
