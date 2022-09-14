#ifndef __ASYNC_THREAD_H__
#define __ASYNC_THREAD_H__

#include <memory>

#include "async_thread_p.hpp"

namespace protoa
{
    class async_thread
    {
    public:
        async_thread();
        async_thread(const async_thread& at);
        async_thread(async_thread&& at);
        ~async_thread();

        void start();
        void stop();
        asio::io_context& ctx();

        async_thread& operator=(const async_thread& at);
        async_thread& operator=(async_thread&& at);

    private:
        std::shared_ptr<async_thread_impl> m_thread;
    };

} // protoa

#endif // __ASYNC_THREAD_H__
