#ifndef __HDK_SINGLETON_HPP__
#define __HDK_SINGLETON_HPP__

#include <atomic>
#include <mutex>

#define HDK_SINGLETON(CLASS_NAME,...) \
public: \
    static CLASS_NAME* instance() \
    { \
        static std::atomic<CLASS_NAME*> s_object {nullptr}; \
        static std::mutex s_mtx; \
        CLASS_NAME* singleton_ = s_object.load(); \
        if ( !singleton_ ) \
        { \
            std::lock_guard<std::mutex> lock_(s_mtx); \
            singleton_ = s_object.load(); \
            if( !singleton_ ) \
            { \
                singleton_ = new CLASS_NAME(__VA_ARGS__); \
                s_object.store(singleton_); \
            } \
        } \
    return s_object; \
    }

#endif//__HDK_SINGLETON_HPP__


