#ifndef __ASYNC_OBJECT_H__
#define __ASYNC_OBJECT_H__

#include "async_thread.hpp"

#include <memory>
#include <mutex>
#include <condition_variable>

namespace protoa
{
    template<typename T>
    class async_object
    {
    public:
        async_object() {}

        template<typename... Args>
        static async_object<T> create(const async_thread& thrd, Args&&... args)
        {
            async_object<T> obj;
            obj.m_object_ptr = std::make_shared<T>(std::forward<Args>(args)...);
            obj.m_async_thread = thrd;
            return obj;
        }

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

} // protoa

#endif // __ASYNC_OBJECT_H__
