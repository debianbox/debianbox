#ifndef __PROTOA_SINGLETON_H__
#define __PROTOA_SINGLETON_H__

#include <atomic>
#include <mutex>

#define PROTOA_SINGLETON(CLASS_NAME,...) \
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



#endif//__PROTOA_SINGLETON_H__


