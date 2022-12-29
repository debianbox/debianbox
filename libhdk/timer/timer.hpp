#ifndef __HDK_TIMER_HPP__
#define __HDK_TIMER_HPP__

#include "asio/io_context.hpp"
#include "asio/steady_timer.hpp"

#include <chrono>
#include <iostream>
#include <functional>
#include <memory>

#include "helpers/helpers.hpp"

namespace hdk
{
    class timer
    {
    public:
        timer(asio::io_context &ctx) :
            m_timer(ctx) {}

        timer(const timer&) = delete;
        timer(timer&&) = delete;
        timer& operator=(timer&&) = delete;
        timer& operator=(const timer&) = delete;

        template<typename Func>
        void timeout(Func func)
        {
            m_timeout_cb = func;
        }

        void single_shot(bool ena)
        {
            m_single_shot = ena;
        }

        bool single_shot()
        {
            return m_single_shot;
        }

        void interval(std::chrono::milliseconds interval)
        {
            m_interval = interval;
        }

        int interval()
        {
            return (int)m_interval.count();
        }

        void start()
        {
            m_timer.expires_from_now(m_interval);
            m_timer.async_wait([this](const asio::error_code& ec) { on_timeout(ec); });
        }

        void start(std::chrono::milliseconds interval)
        {
            m_interval = interval;
            start();
        }

        void stop()
        {
            m_timer.cancel();
        }

    private:
        asio::steady_timer m_timer;
        std::chrono::milliseconds m_interval;
        std::function<void()> m_timeout_cb;
        bool m_single_shot = false;

        void on_timeout(const asio::error_code& ec)
        {
            if(ec)
            {
                if(ec.value() == asio::error::operation_aborted)
                    return;

                std::cerr << "timer error:"<< ec << " " << ec.message() << "\n";
                return;
            }

            m_timeout_cb();
            if(!m_single_shot)
            {
                m_timer.expires_from_now(m_interval);
                m_timer.async_wait([this](const asio::error_code& ec) { on_timeout(ec); });
            }
        }

    public:
        template<typename F, typename ...Args>
        static void single_shot(asio::io_context &ctx, std::chrono::milliseconds interval, F fptr, Args&&... args)
        {
            std::shared_ptr<asio::steady_timer> ptimer = std::make_shared<asio::steady_timer>(ctx);
            ptimer->expires_from_now(interval);
            ptimer->async_wait([ptimer, fptr, args...](const asio::error_code& ec) mutable
                               {
                                   HDK_UNUSED(ec)
                                   (*fptr)(std::forward<Args>(args)...);
                                   ptimer.reset();
                               });
        }

        template<typename Obj, typename F, typename ...Args>
        static void single_shot(asio::io_context &ctx, std::chrono::milliseconds interval, F mfptr, Obj* pobj, Args&&... args)
        {
            std::shared_ptr<asio::steady_timer> ptimer = std::make_shared<asio::steady_timer>(ctx);
            ptimer->expires_from_now(interval);
            ptimer->async_wait([ptimer, mfptr, pobj, args...](const asio::error_code& ec) mutable
                               {
                                   HDK_UNUSED(ec)
                                   (pobj->*mfptr)(std::forward<Args>(args)...);
                                   ptimer.reset();
                               });
        }
    };
} // namespace hdk

#endif // __HDK_TIMER_HPP__
