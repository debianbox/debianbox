#ifndef ASYNC_THREAD_P_HPP
#define ASYNC_THREAD_P_HPP

#include "asio.hpp"

#include <iostream>

namespace protoa
{
    class async_thread_impl
    {
    public:
        async_thread_impl() = default;

        void start()
        {
            auto t = std::thread([this]
                                 {
                                     asio::executor_work_guard<asio::io_context::executor_type> work_guard_local(m_ctx.get_executor());
                                     m_ctx.run();
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
}

#endif // ASYNC_THREAD_P_HPP
