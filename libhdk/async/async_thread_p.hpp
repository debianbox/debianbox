#ifndef __HDK_ASYNC_THREAD_P_HPP__
#define __HDK_ASYNC_THREAD_P_HPP__

#include "asio/io_context.hpp"

#include <unordered_map>
#include <shared_mutex>
#include <iostream>
#include <thread>

#include "singleton.hpp"

namespace hdk
{
    class async_thread_pool
    {
    public:
        HDK_SINGLETON(async_thread_pool)
        void add(std::thread::id thread_id, asio::io_context& ctx)
        {
            std::unique_lock lock(m_mtx);
            m_thread_ctx.insert({thread_id, &ctx});
        }

        void remove(std::thread::id thread_id)
        {
            std::unique_lock lock(m_mtx);
            m_thread_ctx.erase(thread_id);
        }

        asio::io_context& ctx(std::thread::id thread_id) const
        {
            std::shared_lock lock(m_mtx);
            return *m_thread_ctx.at(thread_id);
        }

    private:
        mutable std::shared_mutex m_mtx;
        std::unordered_map<std::thread::id, asio::io_context*> m_thread_ctx;
    };

    class async_thread_impl
    {
    public:
        async_thread_impl() = default;

        void start()
        {
            auto t = std::thread([this]
                                 {
                                     asio::executor_work_guard<asio::io_context::executor_type> work_guard_local(m_ctx.get_executor());
                                     async_thread_pool::instance()->add(std::this_thread::get_id(), m_ctx);
                                     m_ctx.run();

                                     async_thread_pool::instance()->remove(std::this_thread::get_id());
                                 });
            t.detach();
        }

        void stop()
        {
            m_ctx.stop();
        }

        asio::io_context& ctx()
        {
            return m_ctx;
        }

    private:
        asio::io_context m_ctx;
    };

} // namespace hdk

#endif // __HDK_ASYNC_THREAD_P_HPP__
