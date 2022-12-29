#ifndef __HDK_ASYNC_THREAD_HPP__
#define __HDK_ASYNC_THREAD_HPP__

#include <memory>

#include "asio/io_context.hpp"

#include "async/async_thread_p.hpp"

namespace hdk
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
        asio::io_context& ctx() const;

        async_thread& operator=(const async_thread& at);
        async_thread& operator=(async_thread&& at);

        static asio::io_context& this_thread_ctx()
        {
            return async_thread_pool::instance()->ctx(std::this_thread::get_id());
        }

    private:
        std::shared_ptr<async_thread_impl> m_thread;
    };

} // namespace hdk

#endif // __HDK_ASYNC_THREAD_HPP__
