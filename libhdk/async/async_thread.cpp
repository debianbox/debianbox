#include "async/async_thread.hpp"
#include "async/async_thread_p.hpp"

#include <thread>
#include <iostream>

namespace hdk
{
    async_thread::async_thread()
    {
        m_thread = std::make_shared<async_thread_impl>();
    }

    async_thread::async_thread(const async_thread &at)
    {
        m_thread = at.m_thread;
    }

    async_thread::async_thread(async_thread &&at)
    {
        m_thread = at.m_thread;
        at.m_thread.reset();
    }

    async_thread::~async_thread()
    {
        m_thread.reset();
    }

    void async_thread::start()
    {
        m_thread->start();
    }

    void async_thread::stop()
    {
        m_thread->stop();
    }

    asio::io_context &async_thread::ctx() const
    {
        return m_thread->ctx();
    }

    async_thread &async_thread::operator=(const async_thread &at)
    {
        m_thread = at.m_thread;
        return *this;
    }

    async_thread &async_thread::operator=(async_thread &&at)
    {
        m_thread = at.m_thread;
        at.m_thread.reset();
        return *this;
    }

} // namespace hdk
