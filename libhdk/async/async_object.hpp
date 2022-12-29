#ifndef __HDK_ASYNC_OBJECT_H__
#define __HDK_ASYNC_OBJECT_H__

#include <memory>
#include <mutex>
#include <condition_variable>

#include "asio/dispatch.hpp"

#include "async/async_thread.hpp"

namespace hdk
{
    template<typename T>
    class async_object
    {
    public:
        async_object() {}

        template<typename TT, typename... Args>
        friend async_object<TT> async_object_create(const async_thread& thrd, Args&&... args);

        T* addr()
        {
            m_object_ptr.get();
        }

        template<typename F, typename... Args>
        void call_async(F mfptr, Args&&... args)
        {
            asio::dispatch(m_async_thread.ctx(),
                           [this, mfptr, args...]
            {
                ((*m_object_ptr.get()).*mfptr)(args...);
            });
        }

        template<typename F, typename... Args>
        void call_sync(F mfptr, Args&&... args)
        {
            std::mutex mtx;
            std::condition_variable cv;

            asio::dispatch(m_async_thread.ctx(),
                           [this, mfptr, &mtx, &cv, &args...]
            {
                std::unique_lock<std::mutex> lk(mtx);

                ((*m_object_ptr.get()).*mfptr)(args...);

                lk.unlock();
                cv.notify_one();
            });

            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk);
        }

        async_object& operator=(const async_object& obj)
        {
            m_object_ptr = obj.m_object_ptr;
            m_async_thread = obj.m_async_thread;
        }

    private:
        std::shared_ptr<T> m_object_ptr;
        async_thread m_async_thread;
    };

    template<typename TT, typename... Args>
    inline async_object<TT> async_object_create(const async_thread& thrd, Args&&... args)
    {
        async_object<TT> obj;
        obj.m_object_ptr = std::make_shared<TT>(std::forward<Args>(args)...);
        obj.m_async_thread = thrd;
        return obj;
    }

} // namespace hdk

#endif // __HDK_ASYNC_OBJECT_H__
