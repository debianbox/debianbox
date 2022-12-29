#ifndef __HDK_ASYNC_SLEEP_H__
#define __HDK_ASYNC_SLEEP_H__

#include "asio/io_context.hpp"

#include <chrono>

namespace hdk
{
    void async_sleep(asio::io_context& ctx, std::chrono::milliseconds ms)
    {
        auto start = std::chrono::steady_clock::now();
        while(1)
        {
            ctx.poll();
            std::this_thread::yield();

            auto cur = std::chrono::steady_clock::now();
            std::chrono::duration<float> elapsed = cur - start;

            if(elapsed.count() >= ms.count() / 1000.)
                break;
        }
    }
} // namespace hdk


#endif // __HDK_ASYNC_SLEEP_H__
