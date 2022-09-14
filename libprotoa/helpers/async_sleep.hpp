#ifndef __ASYNC_SLEEP_H__
#define __ASYNC_SLEEP_H__

#include <asio.hpp>

#include <chrono>

namespace protoa
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
}


#endif // __ASYNC_SLEEP_H__
